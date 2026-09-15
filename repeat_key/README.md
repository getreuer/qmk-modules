# Repeat Key

<table>
<tr><td><b>Module</b></td><td><tt>getreuer/repeat_key</tt></td></tr>
<tr><td><b>Version</b></td><td>2026-09-14</td></tr>
<tr><td><b>Maintainer</b></td><td>Pascal Getreuer (@getreuer)</td></tr>
<tr><td><b>License</b></td><td><a href="../LICENSE.txt">Apache 2.0</a></td></tr>
<tr><td><b>Documentation</b></td><td>
<a href="https://getreuer.info/posts/keyboards/repeat-key">https://getreuer.info/posts/keyboards/repeat-key</a>
</td></tr>
</table>

This is a module implementation of QMK core's built-in [Repeat
Key](https://docs.qmk.fm/features/repeat_key) feature (enabled by
`REPEAT_KEY_ENABLE = yes`). It is recommended to use the QMK core
implementation, but perhaps for sake of customization or curiosity, you may use
this module.

**Note:** Do not enable both this module and built-in Repeat Key implementations
at the same time, as they define the same symbols.

## Add Repeat Key to your keymap

Add the following to your `keymap.json`:

```json
{
  "modules": ["getreuer/repeat_key"]
}
```

## Keycodes

| Keycode             | Short Alias | Description                                   |
|---------------------|-------------|-----------------------------------------------|
| `QK_REPEAT_KEY`     | `QK_REP`    | Repeat the last pressed key                   |
| `QK_ALT_REPEAT_KEY` | `QK_AREP`   | Perform the alternate of the last pressed key |

**Repeat Key (`QK_REP`)**: Repeats the action of the last pressed key along with
its active modifiers.

**Alternate Repeat Key (`QK_AREP`)**: Performs the "alternate" key for the last
pressed key (e.g. `KC_PGDN` &rarr; `KC_PGUP`, `KC_LEFT` &rarr; `KC_RGHT`,
`KC_BSPC` &rarr; `KC_DEL`).

## Configuration

* `remember_last_key_user(keycode, record, remembered_mods)`: Customize which
  keys are remembered or modify the remembered modifiers.
* `get_alt_repeat_key_keycode_user(keycode, mods)`: Customize or add custom
  alternate key mappings for `QK_AREP`.
* `#define NO_ALT_REPEAT_KEY`: Disable Alternate Repeat functionality to save
  firmware space.

See the [Repeat Key
documentation](https://getreuer.info/posts/keyboards/repeat-key) for further
details.
