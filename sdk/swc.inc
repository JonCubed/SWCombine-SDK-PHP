<?php
namespace SWCombine\SDK;

use SWCombine\SDK\Web\RequestMethods;
use SWCombine\SDK\OAuth\OAuthToken;
use SWCombine\SDK\OAuth\GrantTypes;
use SWCombine\SDK\OAuth\AuthorisationResult;
use SWCombine\SDK\Web\ContentTypes;

define('OAUTH_ENDPOINT_AUTH', 'http://dev.swcombine.net/ws/oauth2/auth/');
define('OAUTH_ENDPOINT_TOKEN', 'http://dev.swcombine.net/ws/oauth2/token/');

spl_autoload_register(array('SWCombine\SDK\SWC', 'autoload'), true, true);

class SWC {
		
	/**
	 * @var string Id of client used in requests.
	 */
	private $client_id;
	
	/**
	 * @var string Client secret that is used in requests.
	 */
	private $client_secret;
	
	/**
	 * @var string Uri that oauth authenticator will redirect to.
	 */
	private $redirect_uri;

	/**
	 * @var string Indicates if your application needs to access a Resource when the user is not present at the browser.
	 */
	private $access_type;
	
	/**
	 * @var OAuthToken OAuth Token.
	 */
	private $token;
	
	function __construct($clientId, $clientSecret, $redirectUri, $isOffline = false) {
		$this->client_id = $clientId;
		$this->client_secret = $clientSecret;
		$this->redirect_uri = $redirectUri;
		$this->access_type = $isOffline ? "offline" : "online";
	}
		
	static function autoload($class) {
		switch($class) {
			case 'SWCombine\SDK\Web\ContentTypes':
				include_once('ContentTypes.inc');
				break;
				
			case 'SWCombine\SDK\OAuth\OAuthToken':
				include_once('OAuthToken.inc');
				break;
				
			case 'SWCombine\SDK\OAuth\GrantTypes':
				include_once('GrantTypes.inc');
				break;
				
			case 'SWCombine\SDK\Web\RequestMethods':
				include_once('RequestMethods.inc');
				break;
				
				
			case 'SWCombine\SDK\OAuth\AuthorisationResult':
				include_once('AuthorisationResult.inc');
				break;
		}
	}
	
	function get_token() {
		return $this->token;
	}
	
	/**
	 * Attempts authorise process by redirecting the user in the browser.
	 * @param array $scopes List of scopes required by the app for user to authorise.
	 * @param string $state Any state information to be pass back to the app on completion of authorisation.
	 */
	public function AttemptAuthorise(array $scopes, $state) {
		$url = OAUTH_ENDPOINT_AUTH.'?response_type=code'.
									'&client_id='.$this->client_id.
									'&scope='.implode(' ', $scopes).
									'&redirect_uri='.$this->redirect_uri.
									'&state='.urlencode($state).
									'&access_type='.$this->access_type;

		header('Accept: '.ContentTypes::JSON);
		header('location: '.$url);
	}
		
	/**
	 * Parses the query tokens returned during authorisation process
	 *
	 * @param array $queryTokens Query tokens to parse for authorisation.
	 */
	public function ParseUrl(array $queryTokens) {
		$result = AuthorisationResult::Error;
	
		if (!isset($queryTokens['code'])) {
			// app was not authorised for some reason
			if ($queryTokens['error'] == 'access_denied') {
				$result = AuthorisationResult::Denied;
			} else {
				// random error so return description value
				$reason = $queryTokens['description'];
			}
		} else {
			// exchange code for a token
			$this->token = $this->GetToken($queryTokens['code']);

			$result = AuthorisationResult::Authorised;
		}
	
		return $result;
	}
	
	/**
	 * Exchanges access_code for tokens.
	 *
	 * @param string $code Access code to exchange for tokens.
	 * @return If successful, returns tokens that can be used by the app.
	 */
	private function GetToken($code) {
		$values = 	array
				  	(
			  		 	"code" => $code
				  		,"client_id" => $this->client_id
				  		,"client_secret" => $this->client_secret
				  		,"redirect_uri" => $this->redirect_uri
				  		,"grant_type" => GrantTypes::AuthorizationCode
			  			,"access_type" => $this->access_type
		  		  	);

		$response = self::MakeRequest(OAUTH_ENDPOINT_TOKEN, RequestMethods::Post, $values);
		
		if (isset($response->error)) {
			throw new SWCombineException('Failed to get token. Reason: '.$response->error, $response->error);
		}
		
		$token = new OAuthToken($response->expires_in, $response->access_token, isset($response->refresh_token) ? $response->refresh_token : null);
		return $token;
	}
	
	public function RefreshToken(OAuthToken $token) {
		$values = array
				  	(
			  		 	"refresh_token" => $token->get_refresh_token()
				  		,"client_id" => $this->client_id
				  		,"client_secret" => $this->client_secret
				  		,"grant_type" => GrantTypes::RefreshToken
		  		  	);

		$response = self::MakeRequest(OAUTH_ENDPOINT_TOKEN, RequestMethods::Post, $values);

		if (isset($response->error)) {
			throw new SWCombineException('Failed to get token. Reason: '.$response->error, $response->error);
		}
		
		$token = new OAuthToken($response->expires_in, $response->access_token, isset($response->refresh_token) ? $response->refresh_token : null);
		return $token;
	}
	
	/**
	 * Makes a request to the specified uri.
	 *
	 * @param string $uri Uri to make the request to.
	 * @param int $method Http Method to use for the request. @see HTTP_METH_XXXX
	 * @param array $values Any parameters to include in the request, where the Key is the parameter name and Value is the parameter value.
	 * @return If successful returns a instance of TValue
	 */
	public static function MakeRequest($uri, $method, array $values) {
		$body = http_build_query($values);
		$headers = array('accept: '.ContentTypes::JSON);
		
		// open connection
		$ch = curl_init();
						
		if ($method == RequestMethods::Get) {
			// values should be query parameters so update uri
			$uri .= '?'.$body;
			$headers[] = 'Content-type: '.ContentTypes::UTF8;
		} else {
			$headers[] = 'Content-type: '.ContentTypes::FormData;
			$headers[] = 'Content-length: '.strlen($body);
			curl_setopt($ch, CURLOPT_POSTFIELDS, $body);
		}
						
		// set url and headers
		curl_setopt($ch, CURLOPT_URL, $uri);
		curl_setopt($ch, CURLOPT_HEADER, 0);
		curl_setopt($ch, CURLOPT_HTTPHEADER, $headers);
		curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1);
		
		// set the method
		switch ($method) {
			case RequestMethods::Post:
				curl_setopt($ch, CURLOPT_POST, 1);
				break;
				
			case RequestMethods::Put:
				curl_setopt($ch, CURLOPT_PUT, 1);
				break;
				
			case RequestMethods::Delete:
				curl_setopt($ch, CURLOPT_CUSTOMREQUEST, 'DELETE');
				break;
		}
		
		// execute
		$response = curl_exec($ch);
		
		// close connection
		curl_close($ch);
	
		return (object)json_decode($response);
	}
}