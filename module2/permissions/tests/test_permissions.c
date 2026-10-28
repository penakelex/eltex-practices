#include "permission_bits.h"
#include "permission_modifier.h"
#include "permission_stat.h"
#include "unity.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

void setUp() {
}

void tearDown() {
}

void test_parse_octal_basic_run() {
  TEST_ASSERT_TRUE(permission_parse_octal("755").success);
  TEST_ASSERT_EQUAL_UINT32(0755, permission_parse_octal("755").bits);

  TEST_ASSERT_TRUE(permission_parse_octal("0644").success);
  TEST_ASSERT_EQUAL_UINT32(0644, permission_parse_octal("0644").bits);

  TEST_ASSERT_TRUE(permission_parse_octal("4755").success);
  TEST_ASSERT_EQUAL_UINT32(04755, permission_parse_octal("4755").bits);
}

void test_parse_octal_invalid_run() {
  TEST_ASSERT_FALSE(permission_parse_octal("789").success);
  TEST_ASSERT_FALSE(permission_parse_octal("").success);
  TEST_ASSERT_FALSE(permission_parse_octal(nullptr).success);
  TEST_ASSERT_FALSE(permission_parse_octal("12345").success);
}

void test_parse_symbolic_basic_run() {
  TEST_ASSERT_TRUE(permission_parse_symbolic("rwxr-xr-x").success);
  TEST_ASSERT_EQUAL_UINT32(0755, permission_parse_symbolic("rwxr-xr-x").bits);

  TEST_ASSERT_TRUE(permission_parse_symbolic("rw-r--r--").success);
  TEST_ASSERT_EQUAL_UINT32(0644, permission_parse_symbolic("rw-r--r--").bits);

  TEST_ASSERT_TRUE(permission_parse_symbolic("---------").success);
  TEST_ASSERT_EQUAL_UINT32(0, permission_parse_symbolic("---------").bits);
}

void test_parse_symbolic_special_run() {
  TEST_ASSERT_TRUE(permission_parse_symbolic("rwsr-xr-x").success);
  TEST_ASSERT_EQUAL_UINT32(04755, permission_parse_symbolic("rwsr-xr-x").bits);

  TEST_ASSERT_TRUE(permission_parse_symbolic("rwSr-xr-x").success);
  TEST_ASSERT_EQUAL_UINT32(04655, permission_parse_symbolic("rwSr-xr-x").bits);

  TEST_ASSERT_TRUE(permission_parse_symbolic("rwxr-xr-t").success);
  TEST_ASSERT_EQUAL_UINT32(01755, permission_parse_symbolic("rwxr-xr-t").bits);

  TEST_ASSERT_TRUE(permission_parse_symbolic("rwxr-xr-T").success);
  TEST_ASSERT_EQUAL_UINT32(01754, permission_parse_symbolic("rwxr-xr-T").bits);
}

void test_parse_symbolic_invalid_run() {
  TEST_ASSERT_FALSE(permission_parse_symbolic("rwx").success);
  TEST_ASSERT_FALSE(permission_parse_symbolic("rwxrwxrwxrwx").success);
  TEST_ASSERT_FALSE(permission_parse_symbolic(nullptr).success);
  TEST_ASSERT_FALSE(permission_parse_symbolic("zzzzzzzzz").success);
}

void test_parse_auto_run() {
  TEST_ASSERT_EQUAL_UINT32(0755, permission_parse("755").bits);
  TEST_ASSERT_EQUAL_UINT32(0755, permission_parse("rwxr-xr-x").bits);
  TEST_ASSERT_EQUAL_UINT32(04755, permission_parse("4755").bits);
}

void test_format_octal_run() {
  char *s = permission_to_octal(0755);
  TEST_ASSERT_NOT_NULL(s);
  TEST_ASSERT_EQUAL_STRING("755", s);
  free(s);

  s = permission_to_octal(04755);
  TEST_ASSERT_NOT_NULL(s);
  TEST_ASSERT_EQUAL_STRING("4755", s);
  free(s);

  s = permission_to_octal(0);
  TEST_ASSERT_NOT_NULL(s);
  TEST_ASSERT_EQUAL_STRING("000", s);
  free(s);
}

void test_format_symbolic_run() {
  char *s = permission_to_symbolic(0755);
  TEST_ASSERT_NOT_NULL(s);
  TEST_ASSERT_EQUAL_STRING("rwxr-xr-x", s);
  free(s);

  s = permission_to_symbolic(04755);
  TEST_ASSERT_NOT_NULL(s);
  TEST_ASSERT_EQUAL_STRING("rwsr-xr-x", s);
  free(s);

  s = permission_to_symbolic(04644);
  TEST_ASSERT_NOT_NULL(s);
  TEST_ASSERT_EQUAL_STRING("rwSr--r--", s);
  free(s);

  s = permission_to_symbolic(01755);
  TEST_ASSERT_NOT_NULL(s);
  TEST_ASSERT_EQUAL_STRING("rwxr-xr-t", s);
  free(s);
}

void test_format_binary_run() {
  char *s = permission_to_binary(0755);
  TEST_ASSERT_NOT_NULL(s);
  TEST_ASSERT_EQUAL_STRING("000 111 101 101", s);
  free(s);

  s = permission_to_binary(04755);
  TEST_ASSERT_NOT_NULL(s);
  TEST_ASSERT_EQUAL_STRING("100 111 101 101", s);
  free(s);

  s = permission_to_binary(0);
  TEST_ASSERT_NOT_NULL(s);
  TEST_ASSERT_EQUAL_STRING("000 000 000 000", s);
  free(s);
}

void test_modify_add_run() {
  TEST_ASSERT_TRUE(permission_modify(0, "u+x").success);
  TEST_ASSERT_EQUAL_UINT32(0100, permission_modify(0, "u+x").bits);

  TEST_ASSERT_TRUE(permission_modify(0644, "u+x").success);
  TEST_ASSERT_EQUAL_UINT32(0744, permission_modify(0644, "u+x").bits);

  TEST_ASSERT_TRUE(permission_modify(0, "+x").success);
  TEST_ASSERT_EQUAL_UINT32(0111, permission_modify(0, "+x").bits);
}

void test_modify_remove_run() {
  TEST_ASSERT_TRUE(permission_modify(0755, "u-x").success);
  TEST_ASSERT_EQUAL_UINT32(0655, permission_modify(0755, "u-x").bits);

  TEST_ASSERT_TRUE(permission_modify(0777, "a-w").success);
  TEST_ASSERT_EQUAL_UINT32(0555, permission_modify(0777, "a-w").bits);
}

void test_modify_set_run() {
  TEST_ASSERT_TRUE(permission_modify(0777, "u=rw").success);
  TEST_ASSERT_EQUAL_UINT32(0677, permission_modify(0777, "u=rw").bits);

  TEST_ASSERT_TRUE(permission_modify(0777, "u=").success);
  TEST_ASSERT_EQUAL_UINT32(0077, permission_modify(0777, "u=").bits);

  TEST_ASSERT_TRUE(permission_modify(04755, "u=").success);
  TEST_ASSERT_EQUAL_UINT32(0055, permission_modify(04755, "u=").bits);
}

void test_modify_comma_run() {
  TEST_ASSERT_TRUE(permission_modify(0, "u+r,g+w,o+x").success);
  TEST_ASSERT_EQUAL_UINT32(0421, permission_modify(0, "u+r,g+w,o+x").bits);

  TEST_ASSERT_TRUE(permission_modify(0777, "u-x,g-x,o-x").success);
  TEST_ASSERT_EQUAL_UINT32(0666, permission_modify(0777, "u-x,g-x,o-x").bits);
}

void test_modify_special_run() {
  TEST_ASSERT_TRUE(permission_modify(0755, "u+s").success);
  TEST_ASSERT_EQUAL_UINT32(04755, permission_modify(0755, "u+s").bits);

  TEST_ASSERT_TRUE(permission_modify(0755, "g+s").success);
  TEST_ASSERT_EQUAL_UINT32(02755, permission_modify(0755, "g+s").bits);

  TEST_ASSERT_TRUE(permission_modify(0755, "o+t").success);
  TEST_ASSERT_EQUAL_UINT32(01755, permission_modify(0755, "o+t").bits);
}

void test_modify_special_clear_run() {
  TEST_ASSERT_TRUE(permission_modify(04755, "u-s").success);
  TEST_ASSERT_EQUAL_UINT32(00755, permission_modify(04755, "u-s").bits);

  TEST_ASSERT_TRUE(permission_modify(04755, "u=rw").success);
  TEST_ASSERT_EQUAL_UINT32(0655, permission_modify(04755, "u=rw").bits);
}

void test_modify_invalid_run() {
  TEST_ASSERT_FALSE(permission_modify(0, "").success);
  TEST_ASSERT_FALSE(permission_modify(0, "z+x").success);
  TEST_ASSERT_FALSE(permission_modify(0, "u+x,").success);
  TEST_ASSERT_FALSE(permission_modify(0, nullptr).success);
}

void test_stat_file_run() {
  const char *path = "/tmp/eltex_test_permissions_file";
  FILE *f = fopen(path, "w");
  TEST_ASSERT_NOT_NULL(f);
  fclose(f);

  TEST_ASSERT_EQUAL_INT(0, chmod(path, 0640));

  const PermissionStatResult r = permission_stat_file(path);
  TEST_ASSERT_TRUE(r.success);
  TEST_ASSERT_EQUAL_UINT32(0640, r.bits);

  remove(path);
}

void test_stat_missing_run() {
  const PermissionStatResult r =
      permission_stat_file("/tmp/eltex_this_file_does_not_exist_98765");
  TEST_ASSERT_FALSE(r.success);
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_parse_octal_basic_run);
  RUN_TEST(test_parse_octal_invalid_run);
  RUN_TEST(test_parse_symbolic_basic_run);
  RUN_TEST(test_parse_symbolic_special_run);
  RUN_TEST(test_parse_symbolic_invalid_run);
  RUN_TEST(test_parse_auto_run);
  RUN_TEST(test_format_octal_run);
  RUN_TEST(test_format_symbolic_run);
  RUN_TEST(test_format_binary_run);
  RUN_TEST(test_modify_add_run);
  RUN_TEST(test_modify_remove_run);
  RUN_TEST(test_modify_set_run);
  RUN_TEST(test_modify_comma_run);
  RUN_TEST(test_modify_special_run);
  RUN_TEST(test_modify_special_clear_run);
  RUN_TEST(test_modify_invalid_run);
  RUN_TEST(test_stat_file_run);
  RUN_TEST(test_stat_missing_run);
  return UNITY_END();
}