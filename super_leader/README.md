# Super Leader

<table>
<tr><td><b>Module</b></td><td><tt>getreuer/super_leader</tt></td></tr>
<tr><td><b>Version</b></td><td>2026-08-20</td></tr>
<tr><td><b>Maintainer</b></td><td>Pascal Getreuer (@getreuer)</td></tr>
<tr><td><b>License</b></td><td><a href="../LICENSE.txt">Apache 2.0</a></td></tr>
<tr><td><b>Documentation</b></td><td>
<a href="https://getreuer.info/posts/keyboards/super-leader">https://getreuer.info/posts/keyboards/super-leader</a>
</td></tr>
</table>

Super Leader is like QMK's built-in [Leader
Key](https://docs.qmk.fm/features/leader_key), but with two substantial
improvements:

* More responsive: When a sequence is not a prefix of any other sequence, it
  resolves immediately when completed. No waiting for the timeout.

* Sequence definitions are more concise, typically one line per sequence.
  Built-in helpers enable sequences to result in sending a string or Unicode
  string or calling a user-defined function.

## Use

Add the following to your `keymap.json`:

```json
{
    "modules": ["getreuer/super_leader"]
}
```

In your `keymap.c` file, add a `LEADER` key somewhere in your layout. Then in
your keymap folder, add a file `super_leader.def` to define one or more leader
sequences like:

```c
SEQ_KEY(v,   (KC_V),             C(KC_V))
SEQ_KEY(dfu, (KC_D, KC_F, KC_U), QK_BOOT)
SEQ_STR(me,  (KC_M, KC_E),       "my@email.com")
SEQ_UNI(thx, (KC_T, KC_H, KC_X), "🙏")
```

A practical way to fit `LEADER` into a keymap is to assign it in the
<kbd>,</kbd> comma key position. Then include this in `super_leader.def`:

```c
// LEADER acts like KC_COMMA, unless there is a longer matching sequence.
SEQ_KEY(leader_default, (), KC_COMMA)
```

Each "`SEQ_*(name, (key1, key2, ...), output)`" line defines a leader sequence.
The first arg is a unique name for the sequence (any valid C identifier). The
second arg `(key1, key2, ...)` lists the sequence keys. The third arg specifies
the output.

With the above flashed to the keyboard:

* Typing <kbd>LEADER</kbd> followed by <kbd>V</kbd> sends Ctrl+V.
* Typing <kbd>LEADER</kbd>, <kbd>D</kbd>, <kbd>F</kbd> <kbd>U</kbd> sends
  `QK_BOOT`, putting the keyboard into DFU mode.
* Typing <kbd>LEADER</kbd>, <kbd>M</kbd>, <kbd>E</kbd> uses `SEND_STRING()` to
  produce `my@email.com`.
* Typing <kbd>LEADER</kbd>, <kbd>T</kbd>, <kbd>H</kbd>, <kbd>X</kbd> uses
  `send_unicode_string()` to produce 🙏. This assumes a Unicode input mode has
  been configured (see the [Unicode feature
  documentation](https://docs.qmk.fm/features/unicode) for instructions).

When entering a leader sequence, layer switch keys are ignored. This allows for
the possibility of sequences of keys involving more than one layer. Successive
keys of a sequence must be typed within 1000&nbsp;ms of the previous key,
otherwise Super Leader times out (configurable with `SUPER_LEADER_TIMEOUT`).

Leader sequences may be up to maximum of 5 keys in length, not counting the
leader key itself (configurable with `SUPER_LEADER_MAX_LENGTH`).

When evaluating the sequence, tap-hold keys are reduced to their tapping
keycode. E.g. the sequence definition `SUPER_LEADER((KC_D, KC_F, KC_U),
QK_BOOT)` matches even when the <kbd>F</kbd> key is actually a mod-tap like
`LSFT_T(KC_F)`.

## Customization

Optionally, callbacks `super_leader_start_user()` and `super_leader_end_user()`
may be defined to know when a leader sequence starts or ends. E.g. in
`keymap.c`:

```c
void super_leader_start_user(void) {
  // Leader sequence started.
}

void super_leader_end_user(bool successful_match) {
  if (successful_match) {
    // Leader sequence matched.
  } else {
    // Ended without a match.
  }
}
```

Additionally, these functions may be used to interact with Super Leader
programmatically.

| Function                         | Description                             |
|----------------------------------|-----------------------------------------|
| `super_leader_sequence_active()` | Whether a leader sequence is active.    |
| `super_leader_start()`           | Taps `LEADER`, beginning a sequence.    |
| `super_leader_cancel()`          | Cancels the leader sequence, if active. |
| `super_leader_reset_timer()`     | Resets the leader sequence timer.       |
| `super_leader_add(kc)`           | Adds a key to the sequence buffer.      |


See the [Super Leader
documentation](https://getreuer.info/posts/keyboards/super-leader) for further
configuration options and details.

