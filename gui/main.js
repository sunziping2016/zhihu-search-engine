const {app, dialog, BrowserWindow} = require('electron');
const path = require('path');
const url = require('url');
const libquery = require('./query');

require('electron-debug')({enabled: true});

//noinspection JSAnnotator
global.shared = {
    query: new libquery(),
    loaded: null
};

let win;

app.on('ready', () => {
    win = new BrowserWindow({width: 800, height: 600, icon: path.join(__dirname, 'favicon.ico')});
    win.loadURL(url.format({
        pathname: path.join(__dirname, 'index.html'),
        protocol: 'file:',
        slashes: true
    }));
    win.on('closed', () => {
        win = null;
    });
});

app.on('window-all-closed', () => {
    shared.query.destroy();
    app.quit();
});
