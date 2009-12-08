require 'open3'
require 'tempfile'
    GIT_PATH = "/opt/local/bin/git"

    BINARY_FILE_MARKER_FORMAT = /^Cannot display: file marked as a binary type.$/
    GIT_INDEX_MARKER_FORMAT = /^index ([0-9a-f]{40})\.\.([0-9a-f]{40})/

    GIT_BINARY_FILE_MARKER_FORMAT = /^GIT binary patch$/

    GIT_LITERAL_FORMAT = /^literal \d+$/

                when GIT_INDEX_MARKER_FORMAT
                    @git_indexes = [$1, $2]
                when GIT_BINARY_FILE_MARKER_FORMAT
                    @binary = true
                    if (GIT_LITERAL_FORMAT.match(lines[i + 1]) and PrettyPatch.has_image_suffix(@filename)) then
                        @git_image = true
                        startOfSections = i + 1
                    end
                    break
            if @image
                @image_url = "data:image/png;base64," + lines_with_contents.join
            elsif @git_image
                begin
                    raise "index line is missing" unless @git_indexes

                    chunks = nil
                    for i in 0...lines_with_contents.length
                        if lines_with_contents[i] =~ /^$/
                            chunks = [lines_with_contents[i + 1 .. -1], lines_with_contents[0 .. i]]
                            break
                        end
                    end

                    raise "no binary chunks" unless chunks

                    @image_urls = chunks.zip(@git_indexes).collect do |chunk, git_index|
                        FileDiff.extract_contents_from_git_binary_chunk(chunk, git_index)
                    end
                rescue
                    @image_error = "Exception raised during decoding git binary patch:<pre>#{CGI.escapeHTML($!.to_s + "\n" + $!.backtrace.join("\n"))}</pre>"
                end
            end
            elsif @git_image then
                if @image_error
                    str += @image_error
                else
                    for i in (0...2)
                        image_url = @image_urls[i]
                        style = ["remove", "add"][i]
                        str += "<p class=\"#{style}\">"
                        if image_url
                            str += "<img class='image' src='" + image_url + "' />"
                        else
                            str += ["Added", "Removed"][i]
                        end
                    end
                end

        def self.git_new_file_binary_patch(filename, encoded_chunk, git_index)
            return <<END
diff --git a/#{filename} b/#{filename}
new file mode 100644
index 0000000000000000000000000000000000000000..#{git_index}
GIT binary patch
#{encoded_chunk.join("")}literal 0
HcmV?d00001

END
        end

        def self.extract_contents_from_git_binary_chunk(encoded_chunk, git_index)
            # We use Tempfile we need a unique file among processes.
            tempfile = Tempfile.new("PrettyPatch")
            # We need a filename which doesn't exist to apply a patch
            # which creates a new file. Append a suffix so filename
            # doesn't exist.
            filepath = tempfile.path + '.bin'
            filename = File.basename(filepath)

            patch = FileDiff.git_new_file_binary_patch(filename, encoded_chunk, git_index)

            # Apply the git binary patch using git-apply.
            cmd = GIT_PATH + " apply --directory=" + File.dirname(filepath)
            stdin, stdout, stderr = *Open3.popen3(cmd)
            begin
                stdin.puts(patch)
                stdin.close

                error = stderr.read
                raise error if error != ""

                contents = File.read(filepath)
            ensure
                stdin.close unless stdin.closed?
                stdout.close
                stderr.close
                File.unlink(filename) if File.exists?(filename)
            end

            return nil if contents.empty?
            return "data:image/png;base64," + [contents].pack("m")
        end