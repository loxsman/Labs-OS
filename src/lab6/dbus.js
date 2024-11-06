const fs = require('fs');
const path = require('path');
const dbus = require('dbus-native');

const sessionBus = dbus.sessionBus();
if (!sessionBus) {
  console.error('Failed to connect to D-Bus session bus');
  process.exit(1);
}

const INTERFACE_NAME = 'com.example.FileOpener';
const OBJECT_PATH = '/com/example/FileOpener';
const SERVICE_NAME = 'com.example.FileOpenerService';

const fileOpener = {
    OpenFile: function (filePath, callback) {
      const ext = path.extname(filePath).toLowerCase();
      let command;
  
      switch (ext) {
        case '.txt':
          command = `xdg-open ${filePath}`; // текстовый файл
          break;
        case '.jpg':
        case '.png':
          command = `xdg-open ${filePath}`; // изображение
          break;
        case '.pdf':
          command = `xdg-open ${filePath}`; // PDF-файл
          break;
        default:
          return callback(`Unsupported file extension: ${ext}`, ''); // Вернуть сообщение об ошибке
      }
  
      console.log(`Opening file with command: ${command}`);
      require('child_process').exec(command, (err) => {
        if (err) {
          callback(`Error opening file: ${err.message}`, ''); // Возвращаем сообщение об ошибке
        } else {
          callback(null, `File opened successfully: ${filePath}`); // Успешное выполнение
        }
      });
    }
  };
  

sessionBus.requestName(SERVICE_NAME, 0, (err, retCode) => {
  if (err) {
    console.error(`Failed to request name ${SERVICE_NAME}: ${err}`);
    process.exit(1);
  }
  if (retCode !== 1) {
    console.error(`Name ${SERVICE_NAME} is already taken`);
    process.exit(1);
  }

  sessionBus.exportInterface(
    fileOpener,
    OBJECT_PATH,
    {
      name: INTERFACE_NAME,
      methods: {
        OpenFile: ['s', 's']
      }
    }
  );

  console.log(`D-Bus service ${SERVICE_NAME} started`);
});
