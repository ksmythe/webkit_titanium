#!/usr/bin/env python
# Copyright (c) 2009, Google Inc. All rights reserved.
# Copyright (c) 2009 Apple Inc. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met:
# 
#     * Redistributions of source code must retain the above copyright
# notice, this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above
# copyright notice, this list of conditions and the following disclaimer
# in the documentation and/or other materials provided with the
# distribution.
#     * Neither the name of Google Inc. nor the names of its
# contributors may be used to endorse or promote products derived from
# this software without specific prior written permission.
# 
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

import os
import StringIO
import subprocess
import sys

from optparse import make_option

from modules.changelogs import ChangeLog
from modules.logging import error, log, tee
from modules.scm import CommitMessage, detect_scm_system, ScriptError, CheckoutNeedsUpdate
from modules.webkitport import WebKitPort

def commit_message_for_this_commit(scm):
    changelog_paths = scm.modified_changelogs()
    if not len(changelog_paths):
        raise ScriptError(message="Found no modified ChangeLogs, cannot create a commit message.\n"
                          "All changes require a ChangeLog.  See:\n"
                          "http://webkit.org/coding/contributing.html")

    changelog_messages = []
    for changelog_path in changelog_paths:
        log("Parsing ChangeLog: %s" % changelog_path)
        changelog_entry = ChangeLog(changelog_path).latest_entry()
        if not changelog_entry:
            raise ScriptError(message="Failed to parse ChangeLog: " + os.path.abspath(changelog_path))
        changelog_messages.append(changelog_entry)

    # FIXME: We should sort and label the ChangeLog messages like commit-log-editor does.
    return CommitMessage("".join(changelog_messages).splitlines())


class WebKitLandingScripts:
    @staticmethod
    def cleaning_options():
        return [
            make_option("--force-clean", action="store_true", dest="force_clean", default=False, help="Clean working directory before applying patches (removes local changes and commits)"),
            make_option("--no-clean", action="store_false", dest="clean", default=True, help="Don't check if the working directory is clean before applying patches"),
        ]

    @staticmethod
    def build_options():
        return [
            make_option("--ignore-builders", action="store_false", dest="check_builders", default=True, help="Don't check to see if the build.webkit.org builders are green before landing."),
            make_option("--quiet", action="store_true", dest="quiet", default=False, help="Produce less console output."),
            make_option("--non-interactive", action="store_true", dest="non_interactive", default=False, help="Never prompt the user, fail as fast as possible."),
        ] + WebKitPort.port_options()

    @staticmethod
    def land_options():
        return [
            make_option("--no-update", action="store_false", dest="update", default=True, help="Don't update the working directory."),
            make_option("--no-build", action="store_false", dest="build", default=True, help="Commit without building first, implies --no-test."),
            make_option("--no-test", action="store_false", dest="test", default=True, help="Commit without running run-webkit-tests."),
            make_option("--no-close", action="store_false", dest="close_bug", default=True, help="Leave bug open after landing."),
        ]




