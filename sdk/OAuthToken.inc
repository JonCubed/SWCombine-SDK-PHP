<?php
namespace SWCombine\SDK\OAuth;

class OAuthToken {
			
	/**
	 * @var DateTime Date and time when access token will expire.
	 */
	private $expires_at;
	
	/**
	 * @var string Access token used when making a request.
	 */
	private $access_token;
	
	/**
	 * @var string Refresh token used to get renewed Access token.
	 */
	private $refresh_token;

	public function __construct($expiresIn, $accessToken, $refreshToken = null) {
		$now = new \DateTime();
		$this->expires_at = $now->add(new \DateInterval('PT'.$expiresIn.'S'));
		$this->access_token = $accessToken;
		$this->refresh_token = $refreshToken;
	}
	
	/**
	 * Gets the access token.
	 *
	 * @return string Access token used when making a request.
	 */
	public function get_access_token() {
		return $this->access_token;
	}
	
	/**
	 * Gets the refresh token.
	 *
	 * @return string Refresh token used to get renewed Access token.
	 */
	public function get_refresh_token() {
		return $this->refresh_token;
	}
	
	/**
	 * Get remaining life time of token.
	 *
	 * @return int Total remaining life of access token in seconds.
	 */
	public function get_expires_in() {
		$expires_at = $this->get_expires_at();
		$now = new \DateTime();
		$diff = $expires_at->diff($now);

		return $diff->h * 60 * 60 + $diff->i * 60 + $diff->s;
	}
	
	/**
	 * Get date and time when token expires.
	 *
	 * @return DateTime Date and time when access token will expire.
	 */
	public function get_expires_at() {
		return $this->expires_at;
	}
}