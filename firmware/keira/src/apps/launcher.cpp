#include <ff.h>
#include <FS.h>
#include "launcher.h"
#include "appmanager.h"

#include "servicemanager.h"
#include "services/network.h"
#include "services/ftp.h"

#include "wifi_config.h"
#include "demos/lines.h"
#include "demos/disk.h"
#include "demos/ball.h"
#include "demos/transform.h"
#include "demos/cube.h"
#include "demos/epilepsy.h"
#include "demos/letris.h"
#include "demos/keyboard.h"
#include "demos/user_spi.h"
#include "demos/scan_i2c.h"
#include "demos/petpet.h"
#include "demos/combo.h"
#include "gpiomanager.h"
#include "callbacktest.h"
#include "tamagotchi/tamagotchi.h"
#include "lua/luarunner.h"
#include "mjs/mjsrunner.h"
#include "nes/nesapp.h"
#include "weather/weather.h"
#include "modplayer/modplayer.h"
#include "lilcatalog/lilcatalog.h"
#include "liltracker/liltracker.h"
#include "fmanager/fmanager.h"
#include "pastebin/pastebinApp.h"

#include "settings/sound.h"

#include "icons/demos.h"
#include "icons/sdcard.h"
#include "icons/memory.h"
#include "icons/dev.h"
#include "icons/settings.h"
#include "icons/info.h"
#include "icons/app_group.h"

#include <WiFi.h> // for setWiFiTxPower
#include <Preferences.h>

LauncherApp::LauncherApp() : App("Menu") {
    networkService = static_cast<NetworkService*>(ServiceManager::getInstance()->getService<NetworkService>("network"));
}

void LauncherApp::run() {
    for (lilka::Button button : {lilka::Button::UP, lilka::Button::DOWN, lilka::Button::LEFT, lilka::Button::RIGHT}) {
        lilka::controller.setAutoRepeat(button, 10, 300);
    }

    item_t root_item = ITEM::SUBMENU(
        "Головне меню",
        {
            ITEM::SUBMENU(
                "Додатки",
                {
                    ITEM::SUBMENU(
                        "Демо",
                        {
                            ITEM::APP("Лінії", [this]() { this->runApp<DemoLines>(); }),
                            ITEM::APP("Диск", [this]() { this->runApp<DiskApp>(); }),
                            ITEM::APP("Перетворення", [this]() { this->runApp<TransformApp>(); }),
                            ITEM::APP("М'ячик", [this]() { this->runApp<BallApp>(); }),
                            ITEM::APP("Куб", [this]() { this->runApp<CubeApp>(); }),
                            ITEM::APP("Епілепсія", [this]() { this->runApp<EpilepsyApp>(); }),
                            ITEM::APP("PetPet", [this]() { this->runApp<PetPetApp>(); }),
                        },
                        &app_group_img, 0
                    ),
                    ITEM::SUBMENU(
                        "Тести",
                        {
                            ITEM::APP("Клавіатура", [this]() { this->runApp<KeyboardApp>(); }),
                            ITEM::APP("Тест SPI", [this]() { this->runApp<UserSPIApp>(); }),
                            ITEM::APP("I2C-сканер", [this]() { this->runApp<ScanI2CApp>(); }),
                            ITEM::APP("GPIO-менеджер", [this]() { this->runApp<GPIOManagerApp>(); }),
                            ITEM::APP("Combo", [this]() { this->runApp<ComboApp>(); }),
                            ITEM::APP("CallbackTest", [this]() { this->runApp<CallBackTestApp>(); }),
                        },
                        &app_group_img, 0
                    ),
                    ITEM::APP("ЛілКаталог", [this]() { this->runApp<LilCatalogApp>(); }),
                    ITEM::APP("ЛілТрекер", [this]() { this->runApp<LilTrackerApp>(); }),
                    ITEM::APP("Летріс", [this]() { this->runApp<LetrisApp>(); }),
                    ITEM::APP("Тамагочі", [this]() { this->runApp<TamagotchiApp>(); }),
                    ITEM::APP("Погода", [this]() { this->runApp<WeatherApp>(); }),
                    ITEM::APP("Pastebin", [this]() { this->runApp<pastebinApp>(); }),
                },
                &demos_img,
                lilka::colors::Pink
            ),
            ITEM::APP(
                "Браузер SD-карти",
                [this]() { this->runApp<FileManagerApp>(LILKA_SD_ROOT); },
                &sdcard_img,
                lilka::colors::Arylide_yellow
            ),
            ITEM::APP(
                "Браузер SPIFFS",
                [this]() { this->runApp<FileManagerApp>(LILKA_SPIFFS_ROOT); },
                &memory_img,
                lilka::colors::Dark_sea_green
            ),
            ITEM::SUBMENU(
                "Розробка",
                {
                    ITEM::APP("Live Lua", [this]() { this->runApp<LuaLiveRunnerApp>(); }),
                    ITEM::APP("Lua REPL", [this]() { this->runApp<LuaReplApp>(); }),
                },
                &dev_img,
                lilka::colors::Jasmine
            ),
            ITEM::SUBMENU(
                "Налаштування",
                {
                    ITEM::MENU(
                        "WiFi-адаптер",
                        [this]() { this->wifiToggle(); },
                        nullptr,
                        0,
                        [this](void* item) {
                            lilka::MenuItem* menuItem = static_cast<lilka::MenuItem*>(item);
                            menuItem->postfix = networkService->getEnabled() ? "ON" : "OFF";
                        }
                    ),
                    ITEM::MENU("Мережі WiFi", [this]() { this->wifiManager(); }),
                    ITEM::MENU("Потужність WiFi", [this]() { this->setWiFiTxPower(); }),
                    ITEM::MENU("Звук", [this]() { this->runApp<SoundConfigApp>(); }),
                    ITEM::SUBMENU("Сервіси", {
                        ITEM::SUBMENU("FTP", {
                            ITEM::MENU(
                                "Статус",
                                [this]() {
                                    FTPService* ftpService = static_cast<FTPService*>(ServiceManager::getInstance()->getService<FTPService>("ftp"));
                                    ftpService->setEnabled(!ftpService->getEnabled());
                                },
                                nullptr,
                                0,
                                [this](void* item) {
                                    lilka::MenuItem* menuItem = static_cast<lilka::MenuItem*>(item);
                                    FTPService* ftpService = static_cast<FTPService*>(ServiceManager::getInstance()->getService<FTPService>("ftp"));
                                    menuItem->postfix = ftpService->getEnabled() ? "ON" : "OFF";
                                }
                            ),
                            ITEM::MENU(
                                "Користувач",
                                nullptr,
                                nullptr,
                                0,
                                [this](void* item) {
                                    lilka::MenuItem* menuItem = static_cast<lilka::MenuItem*>(item);
                                    FTPService* ftpService = static_cast<FTPService*>(ServiceManager::getInstance()->getService<FTPService>("ftp"));
                                    menuItem->postfix = ftpService->getUser();
                                }
                            ),
                            ITEM::MENU(
                                "Пароль",
                                [this]() {
                                    FTPService* ftpService = static_cast<FTPService*>(ServiceManager::getInstance()->getService<FTPService>("ftp"));
                                    ftpService->createPassword();
                                },
                                nullptr,
                                0,
                                [this](void* item) {
                                    lilka::MenuItem* menuItem = static_cast<lilka::MenuItem*>(item);
                                    FTPService* ftpService = static_cast<FTPService*>(ServiceManager::getInstance()->getService<FTPService>("ftp"));
                                    menuItem->postfix = ftpService->getPassword();
                                }
                            ),
                        }),
                    }),
                    ITEM::MENU("Про систему", [this]() { this->about(); }),
                    ITEM::MENU("Інфо про пристрій", [this]() { this->info(); }),
                    ITEM::MENU("Таблиця розділів", [this]() { this->partitions(); }),
                    ITEM::MENU("Форматування SD-карти", [this]() { this->formatSD(); }),
                    ITEM::MENU("Light sleep", []() { 
                        lilka::board.enablePowerSavingMode();
                        esp_light_sleep_start();
                    }),
                    ITEM::MENU("Deep sleep", []() { 
                        lilka::board.enablePowerSavingMode();
                        esp_deep_sleep_start();
                     }),
                    ITEM::MENU("Перезавантаження", []() {
                        esp_restart();
                     }),
                },
                &settings_img,
                lilka::colors::Orchid
            ),
        }
    );
    showMenu(root_item.name, root_item.submenu, false);
}
void LauncherApp::showMenu(const char* title, ITEM_LIST& list, bool back) {
    int itemCount = list.size();
    lilka::Menu menu(title);
    for (int i = 0; i < list.size(); i++) {
        menu.addItem(list[i].name, list[i].icon, list[i].color);
    }
    if (back) {
        menu.addActivationButton(lilka::Button::B);
        menu.addItem("<< Назад");
    }
    while (1) {
        while (!menu.isFinished()) {
            for (int i = 0; i < list.size(); i++) {
                if (list[i].update != nullptr) {
                    lilka::MenuItem menuItem;
                    menu.getItem(i, &menuItem);
                    list[i].update(&menuItem);
                    menu.setItem(i, menuItem.title, menuItem.icon, menuItem.color, menuItem.postfix);
                }
            }
            menu.update();
            menu.draw(canvas);
            queueDraw();
        }
        if (menu.getButton() == lilka::Button::B) {
            break;
        }
        int16_t index = menu.getCursor();
        if (back && index == itemCount) {
            break;
        }

        item_t item = list[index];
        if (item.callback != nullptr) {
            item.callback();
        }
        if (!item.submenu.empty()) {
            showMenu(item.name, item.submenu);
        }
    }
}
void LauncherApp::alert(String title, String message) {
    lilka::Alert alert(title, message);
    alert.draw(canvas);
    queueDraw();
    while (!alert.isFinished()) {
        alert.update();
        taskYIELD();
    }
}
template <typename T, typename... Args>
void LauncherApp::runApp(Args&&... args) {
    AppManager::getInstance()->runApp(new T(std::forward<Args>(args)...));
}
void LauncherApp::setWiFiTxPower() {
    // String names[16];
    // int partitionCount = lilka::sys.get_partition_labels(names);

    String names[] = {
        "19.5 dBm", "19 dBm", "18.5 dBm", "17 dBm", "15 dBm", "13 dBm", "11 dBm", "8.5 dBm", "7 dBm", "2 dBm", "-1 dBm"
    };
    wifi_power_t values[] = {
        WIFI_POWER_19_5dBm,
        WIFI_POWER_19dBm,
        WIFI_POWER_18_5dBm,
        WIFI_POWER_17dBm,
        WIFI_POWER_15dBm,
        WIFI_POWER_13dBm,
        WIFI_POWER_11dBm,
        WIFI_POWER_8_5dBm,
        WIFI_POWER_7dBm,
        WIFI_POWER_5dBm,
        WIFI_POWER_2dBm,
        WIFI_POWER_MINUS_1dBm
    };
    lilka::Menu wifiSetTxMenu;
    wifiSetTxMenu.setTitle("Оберіть потужність");
    wifiSetTxMenu.addActivationButton(lilka::Button::B); // Exit
    // Add names
    for (auto i = 0; i < sizeof(names) / sizeof(names[0]); i++)
        wifiSetTxMenu.addItem(names[i]);
    // Perform draw
    while (!wifiSetTxMenu.isFinished()) {
        wifiSetTxMenu.update();
        wifiSetTxMenu.draw(canvas);
        queueDraw();
    }
    auto button = wifiSetTxMenu.getButton();

    if (button == lilka::Button::B) return;

    auto index = wifiSetTxMenu.getCursor();

    // Set power immediately
    WiFi.setTxPower(values[index]);

    // Save value to NVS
    Preferences prefs;
    prefs.begin(WIFI_KEIRA_NAMESPACE, false);
    prefs.putInt("txPower", static_cast<int>(values[index]));
    prefs.end();
}
void LauncherApp::wifiToggle() {
    networkService->setEnabled(!networkService->getEnabled());
}
void LauncherApp::wifiManager() {
    if (!networkService->getEnabled()) {
        alert("Помилка", "Спочатку увімкніть WiFi-адаптер");
        return;
    }
    AppManager::getInstance()->runApp(new WiFiConfigApp());
}
void LauncherApp::about() {
    alert("Keira OS", "by Андерсон & friends");
}
void LauncherApp::info() {
    char buf[256];
    sprintf(
        buf,
        "Модель: %s\n"
        "Ревізія: %d\n"
        "Версія ESP-IDF: %s\n"
        "Частота: %d МГц\n"
        "Кількість ядер: %d\n"
        "IP: %s",
        ESP.getChipModel(),
        ESP.getChipRevision(),
        esp_get_idf_version(),
        ESP.getCpuFreqMHz(),
        ESP.getChipCores(),
        networkService->getIpAddr().c_str()
    );
    alert("Інфо про пристрій", buf);
}
void LauncherApp::partitions() {
    String names[16];
    int partitionCount = lilka::sys.get_partition_labels(names);

    ITEM_LIST partitionsMenu;
    for (int i = 0; i < partitionCount; i++) {
        String partition = names[i];
        partitionsMenu.push_back(ITEM::MENU(names[i].c_str(), [this, partition]() {
            alert(
                partition,
                String("Адреса: 0x") + String(lilka::sys.get_partition_address(partition.c_str()), HEX) + "\n" +
                    "Розмір: 0x" + String(lilka::sys.get_partition_size(partition.c_str()), HEX)
            );
        }));
    }
    showMenu("Таблиця розділів", partitionsMenu);
}
void LauncherApp::formatSD() {
    lilka::Alert confirm(
        "Форматування",
        "УВАГА: Це очистить ВСІ дані з SD-карти!\n"
        "\nПродовжити?\n\nSTART - продовжити\nA - скасувати"
    );
    confirm.addActivationButton(lilka::Button::START);
    confirm.draw(canvas);
    queueDraw();
    while (!confirm.isFinished()) {
        confirm.update();
        taskYIELD();
    }
    if (confirm.getButton() != lilka::Button::START) {
        return;
    }

    lilka::ProgressDialog dialog("Форматування", "Будь ласка, зачекайте...");
    dialog.draw(canvas);
    queueDraw();
    if (!lilka::fileutils.createSDPartTable()) {
        alert(
            "Помилка",
            "Не вдалося створити нову таблицю розділів. "
            "Можливо відсутня карта.\n\n"
            "Систему буде перезавантажено."
        );
        esp_restart();
    }
    if (!lilka::fileutils.formatSD()) {
        this->alert(
            "Помилка",
            "Не вдалося форматувати SD-карту.\n\n"
            "Систему буде перезавантажено."
        );
        esp_restart();
    }
    this->alert(
        "Форматування",
        "Форматування SD-карти завершено!\n\n"
        "Систему буде перезавантажено."
    );
    esp_restart();
}
