# Pascal Getreuer's QMK community modules

(This is not an officially supported Google product.)

![ZSA Voyager](doc/voyager.jpg)

This repo contains my community modules for [Quantum Mechanical Keyboard
(QMK)](https://docs.qmk.fm) firmware. I use most of them myself in [my QMK
keymap](https://github.com/getreuer/qmk-keymap). Who knew a keyboard [could do
so much?](https://getreuer.info/posts/keyboards/tour/index.html)


## License

This repo uses the Apache License 2.0 except where otherwise indicated. See the
[LICENSE file](LICENSE.txt) for details.


## Community modules

| Module                                    | Description                                           |
|-------------------------------------------|-------------------------------------------------------|
| [achordion](./achordion/)                 | Customize the tap-hold decision.                      |
| [custom_shift_keys](./custom_shift_keys/) | Customize what keycode is produced when shifted.      |

TODO: More to come...


## How to use these modules

This repo makes use of the [Community
Modules](https://docs.qmk.fm/features/community_modules) support added in QMK
Firmware 0.28.0, released on 2025-02-27. [Update your QMK set
up](https://docs.qmk.fm/newbs_git_using_your_master_branch#updating-your-master-branch)
to get the latest.

### Install instructions

1. Run this shell command to download these modules, replacing
   `/path/to/qmk_firmware` with the path of your "`qmk_firmware`" folder:


   ```sh
   cd /path/to/qmk_firmware
   mkdir -p modules
   git submodule add https://github.com/getreuer/qmk-modules.git modules/getreuer
   git submodule update --init --recursive
   ```
   
   Or if using [External
   Userspace](https://docs.qmk.fm/newbs_external_userspace), replace the first
   line with `cd /path/to/your/external/userspace`.

2. Add a module in your keymap by writing a file `keymap.json` with the content

   ```json
   {
       "modules": ["getreuer/achordion"]
   }
   ```
   
   Or add multiple modules like:
   
   ```json
   {
       "modules": ["getreuer/achordion", "getreuer/sentence_case"]
   }
   ```

3. Follow the module's documentation for further instruction on how to use it.

4. Compile and flash as usual. If there are build errors, try running `qmk
   clean` and compiling again for a clean build.


### Uninstall instructions

1. Remove the modules from `keymap.json`.

2. Delete the `modules/getreuer` folder.

