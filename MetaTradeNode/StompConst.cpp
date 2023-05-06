namespace metatradenode {
	const char* STOMP_PATH = "ws://47.102.200.110/meta-trade/stomp";

	const char* SUB_INIT = "/meta-trade/post/init";
	const char* SUB_TRADE = "/meta-trade/subscribe/trade";
	const char* SUB_SPAWN = "/meta-trade/subscribe/spawn";
	const char* SUB_JUDGE = "/meta-trade/subscribe/judge";
	const char* SUB_SEMI_SYNC = "/meta-trade/subscribe/semi-sync";
	const char* SUB_SYNC = "/meta-trade/user/meta-trade/subscribe/sync";

	const char* POST_TRADE = "/meta-trade/post/trade";
	const char* POST_PROOF = "/meta-trade/post/proof";
	const char* POST_AGREE = "/meta-trade/post/agree";
	const char* POST_SYNC = "/meta-trade/post/sync";

	const char* NETWORK_VERSION = "00";
}