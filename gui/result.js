/**
 * Created by sun on 12/17/16.
 */
const parsed = require('query-string').parse(location.search);
//$('#search-textarea').val(parsed.query);
query.split_words(parsed.query, (err, res) => {
    $('#search-textarea').val(res.join(' '))
});