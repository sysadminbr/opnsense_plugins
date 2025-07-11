#!/usr/bin/env php
<?php

/**
 *    Copyright (C) 2017 Smart-Soft
 *
 *    All rights reserved.
 *
 *    Redistribution and use in source and binary forms, with or without
 *    modification, are permitted provided that the following conditions are met:
 *
 *    1. Redistributions of source code must retain the above copyright notice,
 *       this list of conditions and the following disclaimer.
 *
 *    2. Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *
 *    THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
 *    INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 *    AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *    AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
 *    OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *    SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *    INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *    CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *    ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *    POSSIBILITY OF SUCH DAMAGE.
 *
 */

require_once('script/load_phalcon.php');

use OPNsense\ProxyUserACL\ProxyUserACL;
use OPNsense\Core\Config;

$mdlProxyUserACL = new ProxyUserACL();
$domain = strtoupper((string) Config::getInstance()->object()->system->domain);

array_map('unlink', glob("/usr/local/etc/squid/ACL_useracl_*.txt"));
array_map('unlink', glob("/usr/local/etc/squid/ACL_address_*.txt"));
foreach ($mdlProxyUserACL->getNodeByReference('general.ACLs.ACL')->getNodes() as $acl) {
    if($acl["SourceType"]["address"]["selected"] == "1") {
        $lines = "";
        foreach($acl["SourceAddress"] as $addr => $val) {
            $lines .= $addr . "\n";
        }
        file_put_contents("/usr/local/etc/squid/ACL_address_" . $acl["Priority"] . ".txt", $lines);
    } else {
        $line = $acl["Name"] . "\n" . ($acl["SourceType"]["user"]["selected"] == "1" ? $acl["Name"] . "@" . $domain . "\n" : "");
        file_put_contents("/usr/local/etc/squid/ACL_useracl_" . $acl["Priority"] . ".txt", $line);
    }


    // categories
    $categories_file = fopen("/usr/local/etc/squid/ACL_categories_" . $acl["Priority"] . ".txt", "w");
    foreach($acl["Categories"] as $category_key => $category) {
        if($category["selected"] == 1) {
	    $line = str_replace("_", " ", str_replace("-", "/", $category_key));
            fwrite($categories_file, $line . "\n");
        }
    }
    //$fclose($categories_file);


}
