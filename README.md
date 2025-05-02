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

Işık özelliklerinin ayarlanması

✔ Phong aydınlatma modeli ile gerçekçi ışık ve gölge uygulamaları

Ambient, Diffuse ve Specular bileşenler

Işık kaynağı ve global ışık yönü

✔ Kişisel veya tematik bir sahne tasarımı ile yaratıcılığı geliştirme

Kapıdan geçildiğinde arka plan değişir. Karanlık yerini gökyüzüne bırakır.

Rastgele dağıtılmış iki farklı ağaç türü ve bir kulübe bulunmaktadır.

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

Karanlık bir ortamda tek bir kapı bulunur.
![image](https://github.com/user-attachments/assets/004b9f4e-13df-4894-8dba-439baef161ae)

Kamerayı kapıya yaklaştırıp içinden geçerseniz, arka plan gökyüzü rengine döner.
![image](https://github.com/user-attachments/assets/af85cdb4-a338-4a8c-b094-4dd236bb705d)

Rastgele dağıtılmış iki tür ağaç ve bir kulübe görünür.
![image](https://github.com/user-attachments/assets/2a45d2e1-adc9-4d21-92d1-daf6b10d714f)

