# BenimDünyamOpenGLProje

Bu proje, OpenGL ve GLFW kullanılarak geliştirilen bir 3D sahne simülasyonudur. Kullanıcı, bir kapıdan geçerek çayırlık bir alana ulaşır ve burada rastgele dağıtılmış ağaçlar, bir kulübe ve dinamik ışıklandırma ile etkileşimli bir ortam keşfeder.

## Proje Hedefleri
✔ OpenGL grafik kütüphanesinin etkin kullanımını öğrenmek

GLFW ile pencere yönetimi

GLEW ile OpenGL fonksiyonlarının yüklenmesi

Vertex Buffer Object (VBO), Vertex Array Object (VAO) ve Element Buffer Object (EBO) kullanımı

✔ Temel şekiller ile sahne oluşturma

Kapı, çayır, ağaçlar (silindirik gövde + koni yaprak ve icosahedron 20 yüzlü yaprak) ve kulübe (üçgen prizma ve küp) modellenmiştir.

İndexed mesh ve normal hesaplama teknikleri kullanılmıştır.

✔ 3D koordinat sistemleri ve dönüşümleri uygulama

Model, View, Projection (MVP) matrisleri ile nesnelerin konumlandırılması

Kamera hareketi (WASD + fare bakışı)

✔ Shader programlama (GLSL) ile renk ve aydınlatma kontrolü

Vertex ve Fragment Shader kullanımı

Işık ve malzeme özelliklerinin ayarlanması

✔ Phong aydınlatma modeli ile gerçekçi ışık ve gölge uygulamaları

Ambient, Diffuse ve Specular bileşenler

Işık kaynağı ve global ışık yönü

✔ Kişisel veya tematik bir sahne tasarımı ile yaratıcılığı geliştirme

Kapıdan geçildiğinde değişen arka plan (siyah → gökyüzü)

Rastgele dağıtılmış ağaçlar ve bir kulübe

## Kurulum ve Çalıştırma
### Gereksinimler
OpenGL 3.3+ destekleyen bir grafik kartı
GLFW, GLEW ve GLM kütüphaneleri

Bağımlılıkları yükleyin.
Bir IDE kullanarak çalıştırın.


## Kullanım Kılavuzu
W, A, S, D → Kamera hareketi

Fare hareketi → Bakış yönünü değiştir

ESC → Uygulamadan çık

### Sahne Özellikleri
Başlangıç Alanı

Karanlık bir ortamda tek bir kapı bulunur.

Kapıdan Geçiş

Kamerayı kapıya yaklaştırıp içinden geçerseniz, arka plan gökyüzü rengine döner.

Çayırlık Alan

Rastgele dağıtılmış iki tür ağaç ve bir kulübe görünür.

