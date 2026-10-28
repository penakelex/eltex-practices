#include "permission_modifier.h"

#include "permission_bits.h"

#include <stddef.h>

static bool is_who(char c) {
  return c == 'u' || c == 'g' || c == 'o' || c == 'a';
}

static bool is_op(char c) {
  return c == '+' || c == '-' || c == '=';
}

static bool is_perm(char c) {
  return c == 'r' || c == 'w' || c == 'x' || c == 'X' || c == 's' || c == 't';
}

static const char *apply_clause(uint32_t *bits, const char *cmd) {
  const char *start = cmd;

  uint32_t who_mask = 0;
  bool explicit_who = false;

  while (is_who(*cmd)) {
    explicit_who = true;

    switch (*cmd) {
    case 'u': {
      who_mask |= 1u << 2;
      break;
    }
    case 'g': {
      who_mask |= 1u << 1;
      break;
    }
    case 'o': {
      who_mask |= 1u << 0;
      break;
    }
    case 'a': {
      who_mask = 0x7u;
      break;
    }
    }

    cmd++;
  }

  if (!explicit_who) {
    who_mask = 0x7u;
  }

  bool applied = false;

  while (is_op(*cmd)) {
    const char op = *cmd;
    cmd++;

    uint32_t perm_mask = 0;
    uint32_t has_s = 0;
    uint32_t has_t = 0;

    while (is_perm(*cmd)) {
      switch (*cmd) {
      case 'r': {
        perm_mask |= 0b100u;
        break;
      }
      case 'w': {
        perm_mask |= 0b010u;
        break;
      }
      case 'x':
      case 'X': {
        perm_mask |= 0b001u;
        break;
      }
      case 's': {
        has_s = 1;
        break;
      }
      case 't': {
        has_t = 1;
        break;
      }
      }

      cmd++;
    }

    for (int i = 0; i < 3; i++) {
      if (!(who_mask & (1u << i))) {
        continue;
      }

      const uint32_t shift = (uint32_t)(i * 3);
      const uint32_t class_mask = 0x7u << shift;

      uint32_t special_bits = 0;

      if (has_s) {
        if (i == 2) {
          special_bits |= 1u << 11;
        } else if (i == 1) {
          special_bits |= 1u << 10;
        }
      }

      if (has_t) {
        if (i == 0) {
          special_bits |= 1u << 9;
        }
      }

      uint32_t class_special = 0;

      if (i == 2) {
        class_special = 1u << 11;
      } else if (i == 1) {
        class_special = 1u << 10;
      } else if (i == 0) {
        class_special = 1u << 9;
      }

      switch (op) {
      case '+': {
        *bits |= (perm_mask << shift) | special_bits;
        break;
      }
      case '-': {
        *bits &= ~((perm_mask << shift) | special_bits);
        break;
      }
      case '=': {
        *bits = (*bits & ~class_mask) | (perm_mask << shift);
        *bits = (*bits & ~class_special) | special_bits;
        break;
      }
      }
    }

    applied = true;
  }

  if (!applied) {
    return nullptr;
  }

  if (cmd == start) {
    return nullptr;
  }

  return cmd;
}

PermissionModifyResult permission_modify(uint32_t current_bits,
                                         const char *command) {
  PermissionModifyResult result = {.success = false, .bits = 0};

  if (command == nullptr || command[0] == '\0') {
    return result;
  }

  uint32_t bits = current_bits & PERMISSION_MASK;
  const char *cursor = command;

  while (*cursor != '\0') {
    cursor = apply_clause(&bits, cursor);

    if (cursor == nullptr) {
      return result;
    }

    if (*cursor == ',') {
      cursor++;

      if (*cursor == '\0') {
        return result;
      }
    } else if (*cursor != '\0') {
      return result;
    }
  }

  result.success = true;
  result.bits = bits;
  return result;
}