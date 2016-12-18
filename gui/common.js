/**
 * Created by sun on 12/17/16.
 */

const $ = require('jquery');
const electron = require('electron');
const shared = electron.remote.getGlobal('shared');
const query = shared.query;
const nprogress = require('nprogress');
const noty = (options) => {require('noty')({
    layout: 'topRight',
    theme: 'metroui',
    type: options.type || 'alert',
    animation: {
        open: {height: 'toggle'},
        close: {height: 'toggle'},
        easing: 'swing',
        speed: 200 // opening & closing animation speed
    },
    //theme: 'metroui',
    text: options.text,
    timeout: options.timeout
})};

