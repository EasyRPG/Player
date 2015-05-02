<?php                                                                                                                                                                         
/* Licensed under WTFPL. Do what you want. */

//if (empty($_GET)) exit(0);
function x($s) { printf('<pre>%s</pre>', print_r($s, true)); }

$GAME = 'default';
if (isset($_GET['game'])) {
    if (strpos($_GET['game'],'.') !== false) {
        exit('PANIC! Bad game!');
    }
    if (strpos($_GET['game'],'/') !== false) {
        exit('MANY PANIC! Bad game!');
    }
    if (strpos($_GET['game'],'\\') !== false) {
        exit('MORE PANIC! Bad game!');
    }
    if (strpos($_GET['game'],'cache') !== false) {
        exit('MOST PANIC! Bad game!');
    }
    $GAME = $_GET['game'];
}

define('CACHE_FILE', __DIR__ . '/cache/' . $GAME . '.json');
define('BASE_DIR', __DIR__ . '/' . $GAME);

function storeList(&$store, $dir = '.') {
    foreach(scandir(BASE_DIR . '/'. $dir) as $i) {
        if (in_array($i, array('.','..'))) continue;
        elseif (is_dir(BASE_DIR . '/' . $dir . '/' . $i))
            storeList($store, ($dir == '.') ? $i : $dir . '/' . $i);
        elseif (is_file(BASE_DIR . '/' . $dir . '/' . $i)) { 
            $pos = strrpos($i, '.');
            $fn = ($dir === '.' || $pos === false) ? './' . $i : $dir . '/' . substr($i, 0, $pos);
            $store[strtolower($fn)] = $dir . '/' . $i; 
        }
    }   
}
function updateCache() {
    $store = array();
    storeList($store);
    file_exists(CACHE_FILE) && (unlink(CACHE_FILE) || exit('PANIC! Cache not writable!'));
    file_put_contents(CACHE_FILE, json_encode($store)) !== false || exit('MORE PANIC! Cache not writable!');
    echo 'Cache updated.<br />';
}

if (!is_file(__DIR__ . '/cache')) {
    mkdir(__DIR__ . '/cache');
}

if (!is_dir(__DIR__ . '/cache')) {
    exit("PANIC! Create a directory 'cache'!");
}

if (!is_dir(BASE_DIR)) {
    exit("PANIC! Game not found!");
}

if (isset($_GET['update']) || !file_exists(CACHE_FILE)) {
    updateCache();
    return;
}

$db = json_decode(file_get_contents(CACHE_FILE), true);
//x($db);
if (isset($_GET['file'])) {
    $file = strtolower($_GET['file']);
    if (isset($db[$file])) {
    $url = $GAME . '/' . $db[$file];
        header('Location: ' . $url);
        return;
    }
}
header("HTTP/1.1 404 Not Found");

?>
