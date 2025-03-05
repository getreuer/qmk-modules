# Keycode String

This is a community module adaptation of Keycode String, a utility to convert
QMK keycodes to human-readable strings. It's much nicer to read names like
"`LT(2,KC_D)`" than numerical codes like "`0x4207`."

Add the following to your `keymap.json`:

```json
{
    "modules": ["getreuer/keycode_string"]
}
```

Then use `get_keycode_string(keycode)` like:

```c
dprintf("kc=%s\n", get_keycode_string(keycode));
```

Many common QMK keycodes are understood out of the box by
`get_keycode_string()`, but not all. Optionally, use `KEYCODE_STRING_NAMES_USER`
in keymap.c to define names for additional keycodes or override how any keycode
is formatted.

See the [Keycode String
documentation](https://getreuer.info/posts/keyboards/keycode-string) for further
details.

