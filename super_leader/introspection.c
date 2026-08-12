// Copyright 2026 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#define SEQ_KEY(name_, keys_, kc_) \
  static const uint16_t PROGMEM sl_seq_##name_[] = SUPER_LEADER_ARRAY_(keys_);
#define SEQ_STR(name_, keys_, str_)                                            \
  static const uint16_t PROGMEM sl_seq_##name_[] = SUPER_LEADER_ARRAY_(keys_); \
  static const char PROGMEM sl_data_##name_[] = (str_);
#define SEQ_UNI(name_, keys_, str_) \
  static const uint16_t PROGMEM sl_seq_##name_[] = SUPER_LEADER_ARRAY_(keys_);
#define SEQ_FUN(name_, keys_, ...) \
  static const uint16_t PROGMEM sl_seq_##name_[] = SUPER_LEADER_ARRAY_(keys_);
#include "super_leader.def"
#undef SEQ_KEY
#undef SEQ_STR
#undef SEQ_UNI
#undef SEQ_FUN

#define SEQ_KEY(name_, keys_, kc_) \
  {.keys = sl_seq_##name_, .output = SUPER_LEADER_KEY(kc_)},
#define SEQ_STR(name_, keys_, str_)           \
  {.keys = sl_seq_##name_,                    \
   .output = {.fn = super_leader_send_string, \
              .param = {.user_data = (void*)sl_data_##name_}}},
#define SEQ_UNI(name_, keys_, str_) \
  {.keys = sl_seq_##name_, .output = SUPER_LEADER_UNI(str_)},
#define SEQ_FUN(name_, keys_, ...) \
  {.keys = sl_seq_##name_, .output = SUPER_LEADER_FUN(__VA_ARGS__)},

super_leader_sequence_t super_leader_sequences[] = {
#include "super_leader.def"
};
#undef SEQ_KEY
#undef SEQ_STR
#undef SEQ_UNI
#undef SEQ_FUN

uint16_t super_leader_sequence_count_raw(void) {
  return ARRAY_SIZE(super_leader_sequences);
}

__attribute__((weak)) uint16_t super_leader_sequence_count(void) {
  return super_leader_sequence_count_raw();
}

super_leader_sequence_t* super_leader_sequence_get_raw(uint16_t index) {
  if (index >= super_leader_sequence_count_raw()) {
    return NULL;
  }
  return &super_leader_sequences[index];
}

__attribute__((weak)) super_leader_sequence_t* super_leader_sequence_get(
    uint16_t index) {
  return super_leader_sequence_get_raw(index);
}
