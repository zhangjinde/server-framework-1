#include "../BootServer/global.h"
#include "service_center_handler.h"
#include <string.h>

int loadClusterNode(const char* data) {
	cJSON* cjson_cluster_array, *cjson_cluster, *cjson_version;
	cJSON* root = cJSON_Parse(NULL, data);
	if (!root) {
		logErr(ptr_g_Log(), "Config parse extra data error");
		return 0;
	}
	cjson_version = cJSON_Field(root, "version");
	if (!cjson_version) {
		logErr(ptr_g_Log(), "miss field version");
		return 0;
	}
	cjson_cluster_array = cJSON_Field(root, "clusters");
	if (!cjson_cluster_array) {
		logErr(ptr_g_Log(), "miss field cluster");
		return 0;
	}
	for (cjson_cluster = cjson_cluster_array->child; cjson_cluster; cjson_cluster = cjson_cluster->next) {
		Cluster_t* cluster;
		cJSON* name, *socktype, *ip, *port, *hashkey_array;

		name = cJSON_Field(cjson_cluster, "name");
		if (!name || !name->valuestring || !name->valuestring[0])
			continue;
		ip = cJSON_Field(cjson_cluster, "ip");
		if (!ip)
			continue;
		port = cJSON_Field(cjson_cluster, "port");
		if (!port)
			continue;
		socktype = cJSON_Field(cjson_cluster, "socktype");
		hashkey_array = cJSON_Field(cjson_cluster, "hash_key");

		cluster = newCluster();
		if (!cluster)
			continue;
		if (hashkey_array) {
			int hashkey_arraylen = cJSON_Size(hashkey_array);
			if (hashkey_arraylen > 0) {
				int i;
				cJSON* key;
				unsigned int* ptr_key_array = newClusterKeyArray(cluster, hashkey_arraylen);
				if (!ptr_key_array) {
					freeCluster(cluster);
					continue;
				}
				for (i = 0, key = hashkey_array->child; key && i < hashkey_arraylen; key = key->next, ++i) {
					ptr_key_array[i] = key->valueint;
				}
			}
		}
		if (!socktype) {
			cluster->socktype = SOCK_STREAM;
		}
		else {
			cluster->socktype = if_string2socktype(socktype->valuestring);
		}
		strcpy(cluster->ip, ip->valuestring);
		cluster->port = port->valueint;
		if (!regCluster(name->valuestring, cluster)) {
			freeCluster(cluster);
			continue;
		}
	}
	setClusterVersion(cjson_version->valueint);
	cJSON_Delete(root);
	return 1;
}