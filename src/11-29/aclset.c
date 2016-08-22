// aclset.c
   
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/acl.h>
#include <membership.h>
   
#define PROGNAME "aclset"
   
#define EXIT_ON_ERROR(msg, retval) if (retval) { perror(msg); exit((retval)); }
   
int
main(int argc, char **argv)
{
    int           ret, acl_count = 4;
    acl_t         acl;
    acl_entry_t   acl_entry;
    acl_permset_t acl_permset;
    acl_perm_t    acl_perm;
    uuid_t        uu;
   
    if (argc != 2) {
        fprintf(stderr, "usage: %s <file>\n", PROGNAME);
        exit(1);
    }
   
    // translate Unix user ID to UUID
    ret = mbr_uid_to_uuid(getuid(), uu);
    EXIT_ON_ERROR("mbr_uid_to_uuid", ret);
   
    // allocate and initialize working storage for an ACL with acl_count entries
    if ((acl = acl_init(acl_count)) == (acl_t)NULL) {
        perror("acl_init");
        exit(1);
    }
   
    // create a new ACL entry in the given ACL
    ret = acl_create_entry(&acl, &acl_entry);
    EXIT_ON_ERROR("acl_create_entry", ret);
   
    // retrieve descriptor to the permission set in the given ACL entry
    ret = acl_get_permset(acl_entry, &acl_permset);
    EXIT_ON_ERROR("acl_get_permset", ret);
   
    // a permission
    acl_perm = ACL_DELETE;
   
    // add the permission to the given permission set
    ret = acl_add_perm(acl_permset, acl_perm);
    EXIT_ON_ERROR("acl_add_perm", ret);
   
    // set the permissions of the given ACL entry to those contained in this set
    ret = acl_set_permset(acl_entry, acl_permset);
    EXIT_ON_ERROR("acl_set_permset", ret);
   
    // set the tag type (we want to deny delete permissions)
    ret = acl_set_tag_type(acl_entry,  ACL_EXTENDED_DENY);
    EXIT_ON_ERROR("acl_set_tag_type", ret);
   
    // set qualifier (in the case of ACL_EXTENDED_DENY, this should be a uuid_t)
    ret = acl_set_qualifier(acl_entry, (const void *)uu);
    EXIT_ON_ERROR("acl_set_qualifier", ret);
   
    // associate the ACL with the file
    ret = acl_set_file(argv[1], ACL_TYPE_EXTENDED, acl);
    EXIT_ON_ERROR("acl_set_file", ret);
   
    // free ACL working space
    ret = acl_free((void *)acl);
    EXIT_ON_ERROR("acl_free", ret);
   
    exit(0);
}
