/**
 * Created by sun on 12/17/16.
 */

$('#search-form').submit(function(event) {
    if (!shared.dict_loaded || !$('#search-textarea').val())
        event.preventDefault();
});

if (shared.dict_loaded === null) {
    shared.dict_loaded = false;
    let load_dict = () => {query.load_dict((err, res) => {
        if (err) {
            let index = electron.remote.dialog.showMessageBox({
                type: 'error',
                message: err.message,
                buttons: ['Retry', 'Cancel']
            });
            if (index == 0)
                load_dict();
            else
                electron.remote.app.quit();
        } else
            shared.dict_loaded = true;
    })};
    $(load_dict);
}
