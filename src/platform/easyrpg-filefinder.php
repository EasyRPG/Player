<?php                                                                                                                                                                         
//if (empty($_GET)) exit(0);
function x($s) { printf('<pre>%s</pre>', print_r($s, true)); }

$GAME = '';
if (isset($_GET['game'])) {
    if (strpos($_GET['game'],'.') !== false) {
        exit('GAME PANIC!');
    }
    if (strpos($_GET['game'],'/') !== false) {
        exit('MORE GAME PANIC!');
    }
    $GAME = $_GET['game'];
}
else {
    exit();
}

define('CACHE_FILE', __DIR__ . '/cache/' . $GAME . 'cache.json');
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
    file_exists(CACHE_FILE) && (unlink(CACHE_FILE) || exit('PANIC!'));
    file_put_contents(CACHE_FILE, json_encode($store)) !== false || exit('MORE PANIC!');
    echo 'Cache updated.<br />';
}

if (!is_dir(__DIR__ . '/cache')) {
    exit("CACHE PANIC!");
}

if (!is_dir(BASE_DIR)) {
    exit("IO PANIC!");
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
    $url = $_GET['game'] . '/' . $db[$file];
        header('Location: ' . $url);
        return;
    }
}
header("HTTP/1.1 404 Not Found");

?>
