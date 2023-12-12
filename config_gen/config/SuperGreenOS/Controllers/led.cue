package config

_led_conf: [...]

modules leds: _MODULE & {
  init: false
  code: false
}

modules leds fields "fastmode": _INT8 & _NVS & _HTTP_RW & {
  nvs key: "L_FM"
  default: 1
}

modules led: _MODULE & {
  array_len: len(_led_conf)
  required: true
  init_priority: 90
}

modules led fields "\(k)_duty": _INT8 & _NVS & _HTTP_RW & {
  nvs key: "L_\(k)_D"
  default: 0
  write_cb: true
} for k, v in _led_conf

modules led fields "\(k)_type": _INT8 & _NVS & _HTTP_RW & {
  nvs key: "L_\(k)_T"
  _default_var: "LED_FULLSPECTRUM"
  write_cb: true
} for k, v in _led_conf

modules led fields "\(k)_gpio": _INT8 & {
  default: v.gpio
} for k, v in _led_conf

modules led fields "\(k)_box": _INT8 & _NVS & _HTTP_RW & {
  nvs key: "L_\(k)_BOX"
  default: v.box
  write_cb: true
} for k, v in _led_conf

modules led fields "\(k)_dim": _INT8 & _NVS & _HTTP_RW & {
  nvs key: "L_\(k)_DI"
  default: 100
  write_cb: true
} for k, v in _led_conf

modules led fields "\(k)_fade": _INT8 & _NVS & _HTTP_RW & {
  nvs key: "L_\(k)_FADE"
  default: 1
} for k, v in _led_conf

