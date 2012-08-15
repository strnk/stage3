#!/usr/bin/php
<?php

$ADDR = gmp_init($argv[1]);

echo "PML4T: " . get_bits($ADDR, 39, 47) . "\n";
echo "PDPT : " . get_bits($ADDR, 30, 38) . "\n";
echo "PD   : " . get_bits($ADDR, 21, 29) . "\n";
echo "PT   : " . get_bits($ADDR, 12, 20) . "\n";
echo "Off  : " . get_bits($ADDR, 0, 11) . "\n";


function get_bits($N, $lsb, $msb)
{
	$and = gmp_init(0);

	for ($i = $lsb; $i <= $msb; $i++)
		gmp_setbit($and, $i);

	return gmp_strval(gmp_div(gmp_and($N, $and), gmp_pow(2, $lsb)), 16);
}

