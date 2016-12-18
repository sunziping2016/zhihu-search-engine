/**
 * Created by sun on 12/17/16.
 */
let time_begin = performance.now();
const parsed = require('query-string').parse(location.search);
$('#search-textarea').val(parsed.query);

const max_content_length = 140;
const content_before = 20;

query.split_words(parsed.query, (err, res) => {
    let keywords = res, re = new RegExp(keywords.map(x => x.replace(/[\-\[\]\/\{\}\(\)\*\+\?\.\\\^\$\|]/g, "\\$&")).join('|'), 'ig');
    let cvt = x => {
        if (x.length > max_content_length) {
            let index = x.search(re);
            if (index <= content_before)
                x = x.substr(0, max_content_length) + ' ...';
            else if (index <= x.length - max_content_length + content_before)
                x = '... ' + x.substr(index - content_before, max_content_length) + ' ...';
            else
                x = '... ' + x.substr(x.length - max_content_length);
        }
        return x.replace(re, '<em>$&</em>');
    };
    query.query(parsed.query, (err, res) => {
        $('#statistics-count').text(res.length);
        res.forEach((x, i) => {
            query.get_doc(x.id, (err, doc) => {
                $('#result-page-content').append(
                    `<div class="search-result">
                    <h3 class="result-title"><a href="${doc.url}" class="extern">${cvt(doc.headline)} - ${doc.author}</a></h3>
                    <cite class="result-cite">${doc.url}</cite>
                    <p class="result-content">${cvt(doc.content)}</p>
                </div>`
                );
            });
        });
        $(function () {
            $('a.extern').click(function(event) {
                event.preventDefault();
                electron.shell.openExternal($(this).attr('href'));
            });
            let time_end = performance.now();
            $('#statistics-time').text(Math.round((time_end - time_begin) * 1e3) / 1e6);
        });
    });
});
