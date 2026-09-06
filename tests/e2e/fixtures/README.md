Test fikstürleri (oyuncu dosyaları), sunucunun `fwrite_char` çıktısından üretilmiştir.

- `Denemeuc`: seviye 2, parola `sifre123`, yeni `$m1$` özet biçimi.
- `Denemeuc.legacy`: aynı karakter; parola eski `crypt(3)` DES özeti (`De3UwHv1bUbFQ`,
  yine `sifre123`) ve ünvanı ISO-8859-9 ile kodlanmış (` Işıklı Şövalye`). Girişte
  hem parola özetinin yükseltilmesini hem de Latin-5 → UTF-8 dönüşümünü sınar.
