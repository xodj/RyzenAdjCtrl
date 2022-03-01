systemctl stop ryzenctrl-root.service
rm /usr/bin/RyzenCtrl
rm /etc/dbus-1/system.d/ru.ryzenctrl.service.conf
rm /usr/share/applications/RyzenCtrl.desktop
rm /usr/lib/systemd/system/ryzenctrl-root.service
rm /usr/share/icons/hicolor/256x256/apps/amd_icon.png
cp ./Appfolder/RyzenCtrl /usr/bin/RyzenCtrl
cp ./ru.ryzenctrl.service.conf /etc/dbus-1/system.d/ru.ryzenctrl.service.conf
cp ./RyzenCtrl.desktop /usr/share/applications/RyzenCtrl.desktop
cp ./ryzenctrl-root.service /usr/lib/systemd/system/ryzenctrl-root.service
cp ./media/main/amd_icon.png /usr/share/icons/hicolor/256x256/apps/amd_icon.png
systemctl daemon-reload
systemctl start ryzenctrl-root.service
systemctl enable ryzenctrl-root.service
