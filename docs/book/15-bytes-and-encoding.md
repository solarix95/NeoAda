# 15. Bytes and Text Encoding

Text and binary data are different. NeoAda makes that boundary explicit with `String`, `Bytes`, and `Ada.Text.Encoding`.

## Bytes

`Ada.Bytes` provides a binary buffer with length, append, insert, remove, slice, and search operations.

## Encoding text

`Encoding:encode(text, encoding)` converts text to bytes and `Encoding:decode(data, encoding)` converts bytes to text. `String:fromBytes` and `text.toBytes` are convenience wrappers.

## Supported encodings

The README currently lists UTF-8, Latin-1, ASCII, and UTF-16 variants. Choose an encoding explicitly at external boundaries.

## Binary is not text

Decode a byte sequence only when you know it represents text and you know the encoding.

## Exercises

1. Describe `String` vs `Bytes`.
2. Encode and decode UTF-8 with the current interpreter.
