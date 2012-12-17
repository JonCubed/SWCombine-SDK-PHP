<?php
namespace SWCombine\SDK;

class SWCombineWSException extends Exception {
	
	private $error;
	
	public function __construct($message, $error) {
    	parent::__construct($message);
		$this->error = $error;
	}
	
	public function getError() {
		return $this->error;
	}
}