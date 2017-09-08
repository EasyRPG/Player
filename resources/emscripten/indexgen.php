<?php
/* Licensed under WTFPL. Do what you want. */

echo "*******************************************************************";
echo "* Deprecated by `gencache` tool: https://github.com/EasyRPG/Tools *";
echo "*******************************************************************";

define('CACHE_FILE', 'index.json');
define('BASE_DIR', '.');

function storeList(&$store, $dir = '.') {
    foreach(scandir(BASE_DIR . '/'. $dir) as $i) {
        if (in_array($i, array('.','..'))) continue;
        elseif (is_dir(BASE_DIR . '/' . $dir . '/' . $i))
            storeList($store, ($dir == '.') ? $i : $dir . '/' . $i);
        elseif (is_file(BASE_DIR . '/' . $dir . '/' . $i)) { 
            $pos = strrpos($i, '.');
            $fn = ($dir === '.' || $pos === false) ? './' . $i : $dir . '/' . substr($i, 0, $pos);

            $store[strtolower($fn)] = $result = $dir . '/' . $i;
        }
    }
}

function updateCache() {
    $store = array();
    storeList($store);
    file_exists(CACHE_FILE) && (unlink(CACHE_FILE) || report_error('Cache not writable!'));
    file_put_contents(CACHE_FILE, json_encode($store)) !== false || report_error('Cache not writable!');
    echo 'Cache updated.<br />';
}

updateCache();

?>
