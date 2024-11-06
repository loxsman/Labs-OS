#include <dbus/dbus.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void open_file_with_dbus(const char* file_path) {
    DBusError error;
    dbus_error_init(&error);

    // Подключение к системной шине D-Bus
    DBusConnection* conn = dbus_bus_get(DBUS_BUS_SESSION, &error);
    if (dbus_error_is_set(&error)) {
        fprintf(stderr, "Ошибка подключения к D-Bus: %s\n", error.message);
        dbus_error_free(&error);
        return;
    }

    // Отправка сообщения (настройте в зависимости от приложения)
    // Здесь приводится только пример
    DBusMessage* msg = dbus_message_new_method_call(
        "org.freedesktop.ApplicationService", // имя сервиса
        "/org/freedesktop/ApplicationService", // объект пути
        "org.freedesktop.ApplicationInterface", // интерфейс
        "OpenFile" // метод
    );

    dbus_message_append_args(msg, DBUS_TYPE_STRING, &file_path, DBUS_TYPE_INVALID);

    // Отправка сообщения и ожидание ответа
    DBusMessage* reply = dbus_connection_send_with_reply_and_block(conn, msg, -1, &error);
    if (dbus_error_is_set(&error)) {
        fprintf(stderr, "Ошибка отправки сообщения D-Bus: %s\n", error.message);
        dbus_error_free(&error);
    }

    // Освобождение памяти
    dbus_message_unref(msg);
    if (reply) dbus_message_unref(reply);
    dbus_connection_unref(conn);
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Использование: %s <путь_к_файлу>\n", argv[0]);
        return 1;
    }

    open_file_with_dbus(argv[1]);
    return 0;
}
