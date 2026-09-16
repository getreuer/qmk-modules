# Autocorrection

<table>
<tr><td><b>Module</b></td><td><tt>getreuer/autocorrection</tt></td></tr>
<tr><td><b>Version</b></td><td>2026-09-16</td></tr>
<tr><td><b>Maintainer</b></td><td>Pascal Getreuer (@getreuer)</td></tr>
<tr><td><b>License</b></td><td><a href="../LICENSE.txt">Apache 2.0</a></td></tr>
<tr><td><b>Documentation</b></td><td>
<a href="https://getreuer.info/posts/keyboards/autocorrection">https://getreuer.info/posts/keyboards/autocorrection</a>
</td></tr>
</table>

This is a module implementation of QMK core's built-in
[Autocorrect](https://docs.qmk.fm/features/autocorrect) feature (enabled by
`AUTOCORRECT_ENABLE = yes`). It is recommended to use the QMK core
implementation, but perhaps for sake of customization or curiosity, you may use
this module.

**Note:** To avoid conflict, do not enable both this module and built-in
autocorrection implementations at the same time.

## Add Autocorrection to your keymap

Add the following to your `keymap.json`:

```json
{
  "modules": ["getreuer/autocorrection"]
}
```

## Functions

| Function                           | Description                      |
| :--------------------------------- | :------------------------------- |
| `bool is_autocorrection_on(void)`  | True if Autocorrection is active |
| `void autocorrection_on(void)`     | Turns on Autocorrection          |
| `void autocorrection_off(void)`    | Turns off Autocorrection         |
| `void autocorrection_toggle(void)` | Toggles Autocorrection on/off    |

## Changing the Autocorrection Dictionary

1. Edit or create `autocorrection_dict.txt` with entries in the form `typo ->
   correction`:

   ```
   :thier     -> their
   dosen't    -> doesn't
   fitler     -> filter
   ouput      -> output
   widht      -> width
   ```

2. Run `make_autocorrection_data.py` to generate `autocorrection_data.h`:

   ```bash
   python3 make_autocorrection_data.py
   ```

3. Recompile your keymap. The compilation output should include a message like

   ```
   getreuer/autocorrection: Using custom dictionary (71 entries, 1120 bytes).
   ```

   If not, force a clean build by running `qmk clean`, then compiling.

4. Flash the firmware to the keyboard.

See the [Autocorrection
documentation](https://getreuer.info/posts/keyboards/autocorrection) for further
details.
