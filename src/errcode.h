#ifndef ERRCODE_H
#define ERRCODE_H

enum errcode{
	NO_ERR = 0,
        EXIT_ERR,
        PRINT_HELP,
        FILE_NOT_FOUND,
        FILE_LAYOUT_MALFORM,
        FILE_TAGS_MALFORM,
        FILE_TAG_UNSUPP,
        FILE_FORMAT_UNEXP,
        FILE_DUPE_ATTR,
        RULE_PARSE_FAIL,
	OUT_OF_MEM
};

#endif
