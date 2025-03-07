| Supported Targets | ESP32-S3 |
| ----------------- | -------- |

# ESP-IDF BT/BLE HID Device "Secure keyboard"

## How to Use Example
TBD


# TODO

## Pop up menus:
    device tab: 2 btn visible: conn/dis, (...) -> create, delete, set device name
    password tab: 2 btn visible: apply, (...) -> create, delete, edit

## Password edit dialog
    Separate entering name from password into 2 different pages
    always keep focus on text field, always show kb
    use separate dialog for configuring password generation, the button next to GEN
    remove tabs generate/type

## Refactoring
    merge the logic for registry
    use flex everywhere, remove constant positions and sizes

## Misc
    add brightness control
