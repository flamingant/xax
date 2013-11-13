/*
~! use tstat ; !~
~~sopen("Z")~~
~~tsf("queue",		"int")~~
~~tsf("name",			"char *")~~
~~tsf("total_size",		"int")~~
~~tsf("state",		"ENUM_TORRENTSTATE")~~
~~tsf("progress",		"double")~~
~~tsf("num_seeds",		"int")~~
~~tsf("total_seeds",		"int")~~
~~tsf("num_peers",		"int")~~
~~tsf("total_peers",		"int")~~
~~tsf("download_payload_rate","int")~~
~~tsf("upload_payload_rate",	"int")~~
~~tsf("eta",			"int")~~
~~tsf("ratio",		"double")~~
~~sopen("Y")~~
~~tsf("distributed_copies",	"double")~~
~~tsf("is_auto_managed",	"bool")~~
~~tsf("time_added",		"double")~~
~~tsf("tracker_host",		"char *")~~
~~tsf("save_path",		"char *")~~
~~tsf("total_done",		"int64")~~
~~tsf("total_uploaded",	"int64")~~
~~tsf("max_download_speed",	"int")~~
~~tsf("max_upload_speed",	"int")~~
~~tsf("seeds_peers_ratio",	"double")~~
~~tsf("label",		"char *")~~
*/
