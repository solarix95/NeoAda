-- Creates a tiny 1x1 transparent PNG file as raw binary data.
-- Run from the repository root; output: /tmp/neoada_tiny.png

with Ada.Bytes;
with Ada.Io.File;

declare png : Bytes;

-- PNG signature
png.append(137_b);
png.append(80_b);
png.append(78_b);
png.append(71_b);
png.append(13_b);
png.append(10_b);
png.append(26_b);
png.append(10_b);

-- IHDR chunk: 1x1, RGBA, 8 bits per channel
png.append(0_b);
png.append(0_b);
png.append(0_b);
png.append(13_b);
png.append(73_b);
png.append(72_b);
png.append(68_b);
png.append(82_b);
png.append(0_b);
png.append(0_b);
png.append(0_b);
png.append(1_b);
png.append(0_b);
png.append(0_b);
png.append(0_b);
png.append(1_b);
png.append(8_b);
png.append(6_b);
png.append(0_b);
png.append(0_b);
png.append(0_b);
png.append(31_b);
png.append(21_b);
png.append(196_b);
png.append(137_b);

-- IDAT chunk: zlib-compressed transparent pixel
png.append(0_b);
png.append(0_b);
png.append(0_b);
png.append(10_b);
png.append(73_b);
png.append(68_b);
png.append(65_b);
png.append(84_b);
png.append(120_b);
png.append(156_b);
png.append(99_b);
png.append(0_b);
png.append(1_b);
png.append(0_b);
png.append(0_b);
png.append(5_b);
png.append(0_b);
png.append(1_b);
png.append(13_b);
png.append(10_b);
png.append(45_b);
png.append(180_b);

-- IEND chunk
png.append(0_b);
png.append(0_b);
png.append(0_b);
png.append(0_b);
png.append(73_b);
png.append(69_b);
png.append(78_b);
png.append(68_b);
png.append(174_b);
png.append(66_b);
png.append(96_b);
png.append(130_b);

declare fn: String := "/tmp/neoada_tiny.png";
declare f : File := File:create(fn);
f.write(png);
f.close();

print(fn & " created");
