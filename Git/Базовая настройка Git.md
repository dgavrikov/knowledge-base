# Базовая настройка Git

Эта инструкция поможет настроить Git для комфортной работы.

## Шаг 1: Установка имени и email

Установите своё имя и email, чтобы коммиты были правильно подписаны:

```bash
git config --global user.name "User name"
git config --global user.email "your_email@example.com"
```

## Шаг 2: Настройка алиасов

Алиасы упрощают команды Git. Добавьте их глобально:

```bash
git config --global alias.hist "log --pretty=format:\"%C(yellow)%h %C(reset) %C(cyan)%ad %C(reset)| %s %C(green)%d %C(reset) %C(dim white) [%an] %C(reset)\" --graph --date=short"
git config --global alias.co "checkout"
git config --global alias.ci "commit"
git config --global alias.st "status"
git config --global alias.br "branch"
git config --global alias.type "cat-file -t"
git config --global alias.dump "cat-file -p"
```

## Шаг 3: Настройка core параметров

Для корректной работы с путями и кодировками:

```bash
git config --global core.quotepath false
git config --global core.longpaths true
```

## Полный пример конфигурации

Вы можете скопировать эту конфигурацию в файл `~/.gitconfig` (или использовать команды выше):

```
[user]
	name = User name
	email = your_email@example.com
[alias]
	hist = log --pretty=format:\"%C(yellow)%h %C(reset) %C(cyan)%ad %C(reset)| %s %C(green)%d %C(reset) %C(dim white) [%an] %C(reset)\" --graph --date=short
	co = checkout
 	ci = commit
	st = status
	br = branch
	type = cat-file -t
	dump = cat-file -p
[core]
	quotepath = false
	longpaths = true
```

## Проверка настройки

Проверьте конфигурацию:

```bash
git config --list --global
```

Теперь Git готов к работе! Для более детальной информации обратитесь к [официальной документации Git](https://git-scm.com/doc).