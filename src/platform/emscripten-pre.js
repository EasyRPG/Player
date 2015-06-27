function _readdir_r(dirp, entry, result) {
    // int readdir_r(DIR *dirp, struct dirent *entry, struct dirent **result);
    // http://pubs.opengroup.org/onlinepubs/007908799/xsh/readdir_r.html
    var stream = FS.getStreamFromPtr(dirp);
    if (!stream) {
        return ___setErrNo(ERRNO_CODES.EBADF);
    }
    if (!stream.currReading) {
        try {
	    // load the list of entries now, then readdir will traverse that list, to ignore changes to files
	    stream.currReading = FS.readdir(stream.path);
        } catch (e) {
	    return FS.handleFSError(e);
        }
    }
    if (stream.position < 0 || stream.position >= stream.currReading.length) {
        HEAP32[((result)>>2)]=0;
        return 0;
    }
    var id;
    var type;
    var name = stream.currReading[stream.position++];
    if (!name.indexOf('.')) {
        id = 1;
        type = 4;
    } else {
        try {
	    // child may have been removed since we started to read this directory
	    var child = FS.lookupNode(stream.node, name);
        } catch (e) {
	    // skip to the next entry (not infinite since position is incremented until currReading.length)
	    return _readdir_r(dirp, entry, result);
        }
        id = child.id;
        type = FS.isChrdev(child.mode) ? 2 :  // DT_CHR, character device.
	    FS.isDir(child.mode) ? 4 :     // DT_DIR, directory.
	    FS.isLink(child.mode) ? 10 :   // DT_LNK, symbolic link.
	    8;                             // DT_REG, regular file.
    }
    HEAP32[((entry)>>2)]=id;
    HEAP32[(((entry)+(4))>>2)]=stream.position;
    HEAP32[(((entry)+(8))>>2)]=268;
    stringToUTF8(name, entry + 11, 256);
    HEAP8[(((entry)+(10))>>0)]=type;
    HEAP32[((result)>>2)]=entry;
    return 0;
}
