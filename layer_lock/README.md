# Layer Lock

<table>
<tr><td><b>Module</b></td><td><tt>getreuer/layer_lock</tt></td></tr>
<tr><td><b>Version</b></td><td>2026-09-14</td></tr>
<tr><td><b>Maintainer</b></td><td>Pascal Getreuer (@getreuer)</td></tr>
<tr><td><b>License</b></td><td><a href="../LICENSE.txt">Apache 2.0</a></td></tr>
<tr><td><b>Documentation</b></td><td>
<a href="https://getreuer.info/posts/keyboards/layer-lock">https://getreuer.info/posts/keyboards/layer-lock</a>
</td></tr>
</table>

This is a module implementation of QMK core's built-in [Layer
Lock](https://docs.qmk.fm/features/layer_lock) feature (enabled by
`LAYER_LOCK_ENABLE = yes`). It is recommended to use the QMK core
implementation, but perhaps for sake of customization or curiosity, you may use
this module.

**Note:** Do not enable both this module and built-in Layer Lock implementations
at the same time, as they define the same symbols.

## Add Layer Lock to your keymap

Add the following to your `keymap.json`:

```json
{
  "modules": ["getreuer/layer_lock"]
}
```

## Usage

When tapped, `QK_LAYER_LOCK` (short alias `QK_LLCK`) "locks" the highest
active layer so that it stays on without holding a layer switch key.

Layer Lock supports layers activated by:

* `MO(layer)` momentary layer switch
* `LT(layer, key)` layer tap
* `OSL(layer)` one-shot layer
* `TT(layer)` layer tap toggle
* `LM(layer, mod)` layer-mod key (the layer is locked, but not the mods)

Tapping the `QK_LAYER_LOCK` key again, tapping the layer's `MO`/`TT`/`LM` switch
key, or switching layers with `TO(layer)` unlocks and turns off the layer.

## Configuration

* `layer_lock_set_user(locked_layers)`: Callback called when a layer is locked
  or unlocked (e.g. for status LEDs or displays).
* `#define LAYER_LOCK_IDLE_TIMEOUT 60000`: In `config.h`, set an inactivity
  timeout in milliseconds to automatically unlock layers. If not defined, there
  is no idle timeout.

See the [Layer Lock
documentation](https://getreuer.info/posts/keyboards/layer-lock) for further
details.
