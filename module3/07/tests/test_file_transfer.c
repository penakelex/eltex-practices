#define _POSIX_C_SOURCE 202405L

#include "file_transfer.h"

#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "protocol.h"
#include "tcp_socket.h"
#include "unity.h"

static const char source_path[] = "chat_transfer_source.bin";
static const char received_path[] = "chat_transfer_source.bin.received";

static void write_source_file() {
  FILE *file = fopen(source_path, "wb");
  TEST_ASSERT_NOT_NULL(file);

  for (size_t index = 0; index < CHAT_CHUNK_SIZE + 17; index += 1) {
    const uint8_t byte = (uint8_t)(index % 251);
    fwrite(&byte, 1, 1, file);
  }

  fclose(file);
}

static void remove_test_files() {
  remove(source_path);
  remove(received_path);
}

static void drain_packets_into_receiver(int fd, FileReceiver *receiver,
                                        int *meta_count, int *end_count) {
  uint8_t buffer[CHAT_PACKET_BUFFER_SIZE];
  size_t size = 0;

  while (tcp_socket_receive_packet(fd, buffer, sizeof(buffer), &size)) {
    ChatPacket packet = {0};
    TEST_ASSERT_TRUE(chat_packet_deserialize(buffer, size, &packet));

    switch (packet.type) {
    case CHAT_PACKET_FILE_META: {
      *meta_count += 1;
      TEST_ASSERT_TRUE(
          file_receiver_begin(receiver, packet.file_name, packet.file_size));
      break;
    }
    case CHAT_PACKET_FILE_CHUNK: {
      TEST_ASSERT_TRUE(
          file_receiver_append(receiver, packet.chunk_data, packet.chunk_size));
      break;
    }
    case CHAT_PACKET_FILE_END: {
      *end_count += 1;
      TEST_ASSERT_TRUE(file_receiver_finish(receiver));
      return;
    }
    default: {
      TEST_FAIL_MESSAGE("unexpected packet type");
    }
    }
  }

  TEST_FAIL_MESSAGE("stream ended before FILE_END");
}

void test_file_is_transferred_intact() {
  remove_test_files();
  write_source_file();

  int pair[2];
  TEST_ASSERT_EQUAL_INT(0, socketpair(AF_UNIX, SOCK_STREAM, 0, pair));

  TEST_ASSERT_TRUE(file_transfer_send(pair[0], "alice", source_path));

  FileReceiver receiver;
  file_receiver_initialize(&receiver);

  int meta_count = 0;
  int end_count = 0;
  drain_packets_into_receiver(pair[1], &receiver, &meta_count, &end_count);

  TEST_ASSERT_EQUAL_INT(1, meta_count);
  TEST_ASSERT_EQUAL_INT(1, end_count);
  TEST_ASSERT_EQUAL_UINT64(CHAT_CHUNK_SIZE + 17, receiver.received);

  FILE *source = fopen(source_path, "rb");
  FILE *received = fopen(received_path, "rb");
  TEST_ASSERT_NOT_NULL(source);
  TEST_ASSERT_NOT_NULL(received);

  uint8_t source_byte = 0;
  uint8_t received_byte = 0;

  while (fread(&source_byte, 1, 1, source) == 1) {
    TEST_ASSERT_EQUAL_UINT(1, fread(&received_byte, 1, 1, received));
    TEST_ASSERT_EQUAL_UINT8(source_byte, received_byte);
  }

  TEST_ASSERT_EQUAL_UINT(0, fread(&received_byte, 1, 1, received));

  fclose(source);
  fclose(received);
  file_receiver_free(&receiver);
  close(pair[0]);
  close(pair[1]);
  remove_test_files();
}

void test_send_missing_file_fails() {
  TEST_ASSERT_FALSE(file_transfer_send(0, "alice", "no_such_file.bin"));
}

void test_receiver_rejects_append_without_begin() {
  FileReceiver receiver;
  file_receiver_initialize(&receiver);

  const uint8_t data[] = {1};
  TEST_ASSERT_FALSE(file_receiver_append(&receiver, data, sizeof(data)));
  TEST_ASSERT_FALSE(file_receiver_finish(&receiver));

  file_receiver_free(&receiver);
}

void test_file_transfer_run() {
  RUN_TEST(test_file_is_transferred_intact);
  RUN_TEST(test_send_missing_file_fails);
  RUN_TEST(test_receiver_rejects_append_without_begin);
}
