# Speculative Hold

<table>
<tr><td><b>Module</b></td><td><tt>getreuer/speculative_hold</tt></td></tr>
<tr><td><b>Version</b></td><td>2025-07-25</td></tr>
<tr><td><b>Maintainer</b></td><td>Pascal Getreuer (@getreuer)</td></tr>
<tr><td><b>License</b></td><td><a href="../LICENSE.txt">Apache 2.0</a></td></tr>
<tr><td><b>Documentation</b></td><td>
<a href="https://getreuer.info/posts/keyboards/speculative-hold">https://getreuer.info/posts/keyboards/speculative-hold</a>
</td></tr>
</table>

Speculative Hold is now a [QMK
core feature](https://docs.qmk.fm/tap_hold#speculative-hold).

Speculative Hold is a QMK community module that modifies mod-tap `MT` keys. The
mod is applied immediately on key down, eliminating delay, particularly when
using mod-tap keys in tandem with an external mouse. Depending on how the
mod-tap key is settled, the hold action is canceled (when the mod-tap is being
tapped) or allowed to continue (when the mod-tap is being held). 

Even though Speculative Hold applies the modifier early, it does not change how
the tap-hold decision is settled. Speculative Hold should work when used in
combination with QMK's core tap-hold options. I have tested Speculative Hold
particularly together with Permissive Hold + Chordal Hold + Flow Tap.

To use this module, add the following to your `keymap.json`:

```json
{
    "modules": ["getreuer/speculative_hold"]
}
```

By default, Speculative Hold applies to mod-tap keys where the mod is a Ctrl or
Shift mod. Define the `get_speculative_hold()` callback to customize this.

## Configuration

To enable Speculative Hold for some mod-tap keys and not others, define the
callback like this, in your `keymap.c`:

```cc
bool get_speculative_hold(uint16_t keycode, keyrecord_t* record) {
  switch (keycode) {  // Enable speculative holding for these keys.
    case LCTL_T(KC_ESC):
    case LSFT_T(KC_Z):
    case RSFT_T(KC_SLSH):
      return true;
  }
  return false;  // Disable otherwise.
}
```

A potential problem with speculatively-held mods is the ["flashing modifiers"
problem](https://docs.qmk.fm/features/key_overrides#neutralize-flashing-modifiers)
like left GUI opening the start menu when it is not desired.
To solve this, define `DUMMY_MOD_NEUTRALIZER_KEYCODE` in your `config.h` as a
keycode to which no keyboard shortcuts are bound and define `MODS_TO_NEUTRALIZE`
to specify which mods require intervention. Example:

```cc
// Must be a basic, unmodified, HID keycode.
#define DUMMY_MOD_NEUTRALIZER_KEYCODE KC_RIGHT_CTRL

// Neutralize left alt and left GUI (Default value)
#define MODS_TO_NEUTRALIZE { MOD_BIT(KC_LEFT_ALT), MOD_BIT(KC_LEFT_GUI) }
```

For full documentation, see
<https://getreuer.info/posts/keyboards/speculative-hold>

