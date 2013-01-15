<?
#When creating the scanner object, you are required to pass 2 ip-addresses to it in the Internet Protocol dotted for (www.xxx.yyy.zzz).  This set the range of ip-address which will be scanned.  If you only require one ip-address to be scanned then you should pass the same ip-address twice.

$my_scanner = new PortScanner($ip_address1, $ip_address2);

$my_scanner->set_ports("21");
$my_scanner->set_ports("1-25");
$my_scanner->set_ports("1-10,21,80,1024-1030");

# Thirdy you need to set the timeout to wait for a port response, slower network will require a greater timeout delay:
$my_scanner->set_wait(2);

# Fourthly, and last of the parameters to be set, is the scan delay.  This causes the scanner to pause between each port it scans.  The function takes two arguments, both of which are optional, the first being a delay in seconds, the second being a delay in micro-seconds.  If no arugments are sent, the default delay is 0 micro-seconds:
my_scanner->set_delay(0, 5000);

$results = $my_scanner->do_scan();

#[ip_address] -> [port_number] -> [port_name] |-> [port_status]


  foreach($results as $ip=>$ip_results) {
    echo gethostbyaddr($ip)."\n<blockquote>\n";


    foreach($ip_results as $port=>$port_results) {
        echo "\t".$port." : ".$port_results['pname']." : ";
        if ($port_results['status']==1){echo "open";}
        else {echo "closed";}echo "<br>\n";
    }
    echo "</blockquote>\n\n";
  }
?>
