<?
  /*******************************************************************************
   *  Copyright 2002 - Adrian Ritchie :: GrinGod Productions :: aidy@gringod.com
   *  
   *  This software is being release by Adrian Ritchie for educational purposes.
   *  Whilst it may be re-used by other parties, the copy-right remains with
   *  Adrian Ritchie.  Should this code be re-used either whole or in part, this
   *  agreement should be attached.
   *
   *  Dislaimer:
   *    This software is for educational / private network use only.  Do with it 
   *    what you may, but on your own head be it!
   *******************************************************************************/


  class PortScanner {
  	var $start_ip;    /* start of the ip range to scan */
    var $stop_ip;     /* end of the ip range to scan */
    var $current_ip;  /* current ip address being scanned (this is for future features) */
    var $ports;       /* array of ports to be scanned */
    var $wait;        /* how long to wait for a response from the port */
    var $delay;       /* how long to pause between each port */

    /***
     * Function PortScanner()
     * Class constructor.
     * Sets the start and end address of the scan range
     * must be passed... but may be the same.
     * The end address must be greater than the start address.
     ***/
    function PortScanner($start, $stop) {
    	$this->start_ip = ip2long($start);  /* store the start ip address as a long number */
    	$this->stop_ip = ip2long($stop);    /* store the end ip address as a long number */
    }

    /***
     * Function set_ports
     * Adds the passed ports to the array of ports to be scanned.
     * This can either be a single port, a range of ports e.g. 1-90
     * a a combination of both separated by commas (no spaces).
     ***/
    function set_ports($ports) {
    	/* Explode ports into an array based on comma seperation.
         Will create an array even if there is no comma so no need
         to check for an array in following code */
      $ports_array = explode(",",$ports);

      /* loop through array of ports */
      foreach($ports_array as $key=>$val) {
        /* try to explode port range into an array */
      	if(ereg("([0-9]+)\-([0-9]+)",$val, $buff)) {
          /* loop through range to add each port */
      		for($ii=$buff[1]; $ii<=$buff[2]; $ii++) {
      			$this->ports[] = $ii;
          }
        } else {
          /* only one port was sent so add that */
          $this->ports[] = $val;
        }
      }
    }

    /***
     * Function set_wait
     * Sets the time to wait for response from socket.
     * Good object-oriented design doesn't allow access to class
     * attributes, so accessor functions are provided.
     ***/
    function set_wait($wait) {
      $this->wait = $wait;
    }
    
    /***
     * Function set_delay
     * Sets the delay between each port check. Delay is in micro-seconds.
     * Good object-oriented design doesn't allow access to class
     * attributes, so accessor functions are provided.
     ***/
    function set_delay($seconds=0, $microseconds=0) {
    	$this->delay = (1000000*$seconds) + $microseconds;
    }

    /***
     * Function do_scan
     * Does the actual scan, based on the given details (see above functions).
     ***/
    function do_scan() {
      /* Loop through ip addresses.
         This is why ip addresses are stored as long numbers as apposed
         to Internet Protocol dotted addresses */
      for($this->current_ip=$this->start_ip; $this->current_ip<=$this->stop_ip; $this->current_ip++) {
      	/* convert the long number back to a dotted address */
        $ip = long2ip($this->current_ip);
        
        /* loop through the ports and check each */
        foreach($this->ports as $key=>$port) {
          /* for unix systems, this will obtain the name of the service running
             on that port. Win32 systems will just return N/A. */
          if (!getservbyport($port,"tcp")) {$pname = "N/A";}
          else {$pname = getservbyport($port,"tcp");}
          
          /* attempt to open a socket to the port at the current ip address */
          $ptcp = fsockopen($ip, $port, &$errno, &$errstr, $this->wait);
          if($ptcp) {$status=1;} /* return 1 for open port (so users can display their own message */
          else {$status=0;}      /* return 0 for closed port (so users can display their own message */

          /* return the results in a structured multi-dimensioned array
             so the user can choose how to display the results */
          $results["$ip"]["$port"]["pname"] = "$pname";
          $results["$ip"]["$port"]["status"] = "$status";

          /* start the delay before moving on to the next port */
          $this->do_delay($this->delay);
        }
      }

      /* returnt the results to the user */
      Return $results;
    }

    /*** 
     * Function do_delay
     * This pauses execution for the passed length of time (micro-seconds)
     * This allows a delay of less than 1 second for system which do not 
     * support usleep (Win32).
     * This code was post on the PHP manual by "dsc at c2i dot net"
     * I'm not sure if this works or not, so if your running *nix you
     * may want to change this to use usleep();
     ***/
    function do_delay($delay) {
      $start = gettimeofday();
      do{
        $stop = gettimeofday();
        $timePassed = 1000000 * ($stop['sec'] - $start['sec']) + $stop['usec'] - $start['usec'];
      }while  ($timePassed < $delay);
    }
  }

  /* get the target ip address */
  $ip_address = gethostbyname("www.vbweb.co.uk");

  /* set all the require atributes */
  $my_scanner = new PortScanner($ip_address,$ip_address);
  $my_scanner->set_ports("15-25,80,110,3306,1337,666");
  $my_scanner->set_delay(1);
  $my_scanner->set_wait(2);

  /* do the scan and capture the results */
  $results = $my_scanner->do_scan();


  /* display the results - this simply loops through the ip addresses and indents the results */
  foreach($results as $ip=>$ip_results) {
  	echo gethostbyaddr($ip)."\n<blockquote>\n";
    foreach($ip_results as $port=>$port_results) {
    	echo "\t".$port." : ".$port_results['pname']." : ";
      if ($port_results['status']==1){echo "open";}else {echo "closed";}
      echo "<br>\n";
    }
    echo "</blockquote>\n\n";
  }
?>
