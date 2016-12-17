/**
 * Created by sun on 12/17/16.
 */

const $ = require('jquery');
const electron = require('electron');
const shared = electron.remote.getGlobal('shared');
const query = shared.query;

$('a.external').click(function(event) {
    event.preventDefault();
    electron.shell.openExternal($(this).attr('href'));
});

