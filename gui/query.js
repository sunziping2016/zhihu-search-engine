/**
 * Created by sun on 12/17/16.
 */
const path = require('path');
const ref = require('ref');
const ffi = require('ffi');
const libquery = ffi.Library(path.join(__dirname, './libquery.so'), {
    'init':        [ 'pointer', [] ],
    'destroy':     [ 'void',    [ 'pointer'] ],
    'load_dict':   [ 'pointer', [ 'pointer'] ],
    'split_words': [ 'pointer', [ 'pointer', 'string'] ],
    'get_str':     [ 'string',  [ 'pointer'] ],
    'free_str':    [ 'void',    [ 'pointer'] ]
});

function json_wrapper(callback) {
    return (err, res) => {
        if (err)
            callback(err, null);
        let result = JSON.parse(libquery.get_str(res));
        libquery.free_str(res);
        if (result.okay)
            callback(null, result.result);
        else
            callback(new Error(result.message), null);
    };
}

module.exports = class {
    constructor() {
        this.app = libquery.init();
    }
    destroy() {
        libquery.destroy(this.app);
    }
    load_dict(callback) {
        libquery.load_dict.async(this.app, json_wrapper(callback))
    }
    split_words(words, callback) {
        libquery.split_words.async(this.app, words, json_wrapper(callback))
    }
};