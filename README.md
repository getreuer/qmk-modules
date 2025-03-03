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

TODO: List modules here...


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
   
   Or if using [external userspace]() replace the first line with `cd /path/to/your/external/userspace`.

2. Activate a module in your keymap by writing a file `keymap.json` with the
   content

   ```json
   {
       "modules": ["getreuer/custom_shift_keys"]
   }
   ```
   
   Or activate multiple modules like this:
   
   ```json
   {
       "modules": ["getreuer/custom_shift_keys", "getreuer/sentence_case"]
   }
   ```

3. Follow the module's documentation for further instruction on how to use it.

4. Whenever changing modules, run this shell command to ensure a clean build:

   ```sh
   qmk clean
   ```

   Then compile and flash as usual.


### Uninstall instructions

1. Remove the modules from `keymap.json`.

2. Delete the `modules/getreuer` folder.

