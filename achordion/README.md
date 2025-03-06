# Achordion

This is a community module adaptation of
[Achordion](https://getreuer.info/posts/keyboards/achordion), a customizable
"opposite hands" rule implementation for tap-hold keys. Achordion is the
predecessor of QMK core feature [Chordal
Hold](https://docs.qmk.fm/tap_hold#chordal-hold).

Add the following to your `keymap.json` to use Achordion:

```json
{
    "modules": ["getreuer/achordion"]
}
```

Optionally, Achordion can be customized through several callbacks and config
options. See the [Achordion
documentation](https://getreuer.info/posts/keyboards/achordion) for how to do
that and further details.

