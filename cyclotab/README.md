# Cyclotab

<table>
<tr><td><b>Module</b></td><td><tt>getreuer/cyclotab</tt></td></tr>
<tr><td><b>Version</b></td><td>2026-03-17</td></tr>
<tr><td><b>Maintainer</b></td><td>Pascal Getreuer (@getreuer)</td></tr>
<tr><td><b>License</b></td><td><a href="../LICENSE.txt">Apache 2.0</a></td></tr>
<tr><td><b>Documentation</b></td><td>
<a href="https://getreuer.info/posts/keyboards/cyclotab">https://getreuer.info/posts/keyboards/cyclotab</a>
</td></tr>
</table>

This module is aims to make Alt+Tabbing easier. It is my take on a "swapper."
To use this module, add the following to your `keymap.json`:

```json
{
    "modules": ["getreuer/cyclotab"]
}
```

By default, Cyclotab is configured for the Alt+Tab hotkey, activated by keycodes
`A(KC_TAB)` (Alt+Tab) and `S(A(KC_TAB))` (Shift+Alt+Tab). Use one or both of
these keycodes somewhere in your keymap.

To use Cyclotab:

1. Tap `A(KC_TAB)` and/or `S(A(KC_TAB))` any number of times. Even after the key
   is released, Cyclotab continues to hold the Alt mod for a second, allowing
   for multiple taps to navigate among windows.

2. To complete the selection and release the Alt mod, press or release any other
   key such as Space. This key is "consumed" and not sent to the computer, e.g.
   so that a space is not inadvertently typed when switching to your terminal.
   Alternatively, Alt is released automatically after a configurable timeout.


## Customization

To handle hotkeys other than Alt+Tab, define `CYCLOTAB_KEYS` in your config.h,
like

```c
// Handle Command+Tab.
#define CYCLOTAB_KEYS LCMD(KC_TAB)
```

Or list comma-separated keycodes to handle multiple hotkeys:

```c
// Handle Command+Tab and Command+Grave
#define CYCLOTAB_KEYS LCMD(KC_TAB), LCMD(KC_GRAVE)
```

By default, Cyclotab times out and releases held mods after 1000 ms (1 second).
Define `CYCLOTAB_TIMEOUT` in your config.h to set a different value in units of
milliseconds:

```c
#define CYCLOTAB_TIMEOUT 2500  // 2.5 seconds.
```

Setting a value of `0` disables timeout, in which case mods are then held
indefinitely until another key is pressed or released.


See the [Cyclotab
documentation](https://getreuer.info/posts/keyboards/cyclotab) for further
configuration options and details.

