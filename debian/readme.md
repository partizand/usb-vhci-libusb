Инструкции по сборке пакета debian
==================================

Сборка
------

Для сборки нужно перейти на ветвь debian и запустить сборку

```
git checkout debian
gbp buildpackage
```

Если не нужна подпись при сборке (см. gbp.conf)

```
gbp buildpackage --builder='-us -uc -b'
```

Если все хорошо собралось, можно создать итоговую сборку и добавить метку в git

```
gbp buildpackage --git-tag
```

В настройках gbp.conf указана сборка в каталог ../build-area

Получение обновлений из upstream
--------------------------------

Репозиторий upstream git://git.code.sf.net/p/usb-vhci/libusb_vhci

Если там появились обновления, то получаем их

Добавляем удаленный репозиторий upstream (если его нет, список репозиториев `git remote`)

```
git remote add upstream git://git.code.sf.net/p/usb-vhci/libusb_vhci
```

Получаем изменения ветви master

```
git pull upstream master
```

Отправляем изменения на github (для сохранности, делать не обязательно)

```
git push origin master
```

Как это все выглядит в локальном репозитории:

```
Sourcefourge               localgit     github

master (оригинал)    ->    master   ->  master  (копия)
				           debian  <->  debian (оригинал)
```				
				
				

Далее сливаем ветку debian с изменившейся master (т.е. в ветке debian должен появится новый исходный код). И готовим новый релиз debian в этой ветке (правим changelog). Далее сборка.

Можно получить changelog из комитов git (не ясно как о всё это работает, --snapshot - для тестов, --release - для окончательной записи, в любом случае в комит git это все не попадет)

```
gbp dch [--snapshot]
```


