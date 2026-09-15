# Caps Word

<table>
<tr><td><b>Module</b></td><td><tt>getreuer/caps_word</tt></td></tr>
<tr><td><b>Version</b></td><td>2026-09-14</td></tr>
<tr><td><b>Maintainer</b></td><td>Pascal Getreuer (@getreuer)</td></tr>
<tr><td><b>License</b></td><td><a href="../LICENSE.txt">Apache 2.0</a></td></tr>
<tr><td><b>Documentation</b></td><td>
<a href="https://getreuer.info/posts/keyboards/caps-word">https://getreuer.info/posts/keyboards/caps-word</a>
</td></tr>
</table>

This is a module implementation of QMK core's built-in [Caps
Word](https://docs.qmk.fm/features/caps_word) feature (enabled by
`CAPS_WORD_ENABLE = yes`). It is recommended to use the QMK core implementation,
but perhaps for sake of customization or curiosity, you may use this module.

**Note:** Do not enable both this module and built-in Caps Word implementations
at the same time, as they define the same symbols.

## Add Caps Word to your keymap

Add the following to your `keymap.json`:

```json
{
  "modules": ["getreuer/caps_word"]
}
```

## Usage

Caps Word can be activated by tapping the `CW_TOGG` keycode, or programmatically
by calling `caps_word_on()` or `caps_word_toggle()`. Once activated, letters are
automatically shifted to uppercase. Typing non-alpha word breaks (e.g. Space,
Enter, Escape, Tab, punctuation) automatically deactivates Caps Word. Digits,
Backspace, Delete, minus `-`, and underscore `_` continue the word without
shifting.

## Configuration

* `caps_word_press_user(keycode)`: Override which keys continue or stop Caps
  Word, and which keys are shifted.
* `caps_word_set_user(active)`: Callback when Caps Word turns on or off (e.g.
  for status LEDs).
* `#define CAPS_WORD_IDLE_TIMEOUT 5000`: In `config.h`, set an inactivity
  timeout in milliseconds. If not defined, there is no idle timeout.
* `#define CAPS_WORD_INVERT_ON_SHIFT`: In `config.h`, invert shifting when Shift
  is pressed while Caps Word is active.

See the [Caps Word documentation](https://getreuer.info/posts/keyboards/caps-word) for further details.
