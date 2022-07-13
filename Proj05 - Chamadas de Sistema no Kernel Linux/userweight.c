#include <linux/syscalls.h>
#include <linux/user_namespace.h>

int weightUsers[65536] = {[0 ... 65535] = 10};

SYSCALL_DEFINE2(setuserweight, int, uid, int, weight) {
	if(from_kuid_munged(current_user_ns(), current_uid()) != 0 ||
		weight <= 0 || uid < -1 || uid > 65535) {
		return -EACCES;
	}

	if(uid == -1){
		uid = from_kuid_munged(current_user_ns(), current_uid());
	}
    
	weightUsers[uid] = weight;
	return 0;
}

SYSCALL_DEFINE1(getuserweight, int, uid) {
	if(uid < -1 || uid > 65535){
		return  -EINVAL;
	}

	if(uid == -1){
		uid = from_kuid_munged(current_user_ns(), current_uid());
	}

	return weightUsers[uid];
}
