### Инструкция по установке OpenJDK 21 в WSL (Ubuntu или Debian)

Эта инструкция универсальна для WSL на базе Ubuntu или Debian. Она учитывает, что в Debian stable OpenJDK 21 может отсутствовать в стандартных репозиториях, поэтому использует SDKMAN как основной альтернативный способ. Если у вас Ubuntu 22.04+ или Debian testing/unstable, JDK может быть доступен через apt.

---

#### 1. Обновите систему и проверьте доступность OpenJDK 21
```bash
sudo apt update
apt search openjdk-21
```
- Если в выводе есть `openjdk-21-jdk`, переходите к шагу 2.
- Если нет (часто в Debian stable), переходите к шагу 3 (SDKMAN).

#### 2. Установка через apt (если пакет доступен)
```bash
sudo apt install openjdk-21-jdk
```
Переходите к шагу 4.

#### 3. Установка через SDKMAN (рекомендуется, если apt не сработал)
SDKMAN — менеджер версий JDK, который работает в Linux и автоматически настраивает переменные.
```bash
# Установите SDKMAN
curl -s "https://get.sdkman.io" | bash
source "$HOME/.sdkman/bin/sdkman-init.sh"

# Установите OpenJDK 21
sdk install java 21-open
```
SDKMAN сам задаст `JAVA_HOME` и обновит `PATH`.

#### 4. Настройка переменных окружения (если устанавливали через apt)
Если устанавливали через apt (шаг 2), добавьте в `~/.bashrc` (откройте через `nano ~/.bashrc`):
```bash
export JAVA_HOME=/usr/lib/jvm/java-21-openjdk-amd64
export PATH=$JAVA_HOME/bin:$PATH
```
Примените изменения:
```bash
source ~/.bashrc
```

#### 5. Проверка установки
```bash
java -version
javac -version
mvn -v
```
Должно показать версию 21 для JDK и корректно работать с Maven.

---

#### Примечания
- **Если apt не находит пакет в Debian**: Добавьте репозиторий testing (но это может сломать стабильность системы — лучше SDKMAN).
- **Если SDKMAN не работает**: Альтернатива — ручная установка из tarball (скачайте с [jdk.java.net](https://jdk.java.net/21/), распакуйте в `/usr/lib/jvm/` и настройте `JAVA_HOME` как в шаге 4).
- **Версия системы**: Проверьте `lsb_release -a`, чтобы подтвердить Ubuntu/Debian и версию.
- **Проблемы**: Если возникнут ошибки, поделитесь выводом команд.

Эта инструкция объединяет оба подхода для удобства. Если нужно доработать — скажите!