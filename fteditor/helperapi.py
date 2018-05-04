import os
import sys
import subprocess
import shlex
from threading import Timer
import linecache

_bin_folder = "Bin"


if sys.version_info < (3, 0):
    pythonversion2x = True
else:
    pythonversion2x = False


# TODO: migrate all basestring check to this function
def isstring(string_to_check):
    if pythonversion2x:
        return isinstance(string_to_check, basestring)
    else:
        return isinstance(string_to_check, str)


def execute_subprocess_timed(cmd, timeout=5):
    """
    spawn and execute a command.  There is a 5 second timer to prevent the process from hanging.
    The value should be adjusted to the respective task.
    """
    try:
        proc = Popen(shlex.split(cmd), stdout=subprocess.PIPE, stderr=subprocess.PIPE)

        kill_proc = lambda p: p.kill()
        timer = Timer(timeout, kill_proc, [proc])
        try:
            timer.start()
            stdout, stderr = proc.communicate()
        finally:
            timer.cancel()
            return stdout
    except:
        raise
    '''
    except subprocess.CalledProcessError as ex:
        print 'Error encountered(subprocess): ' + str(ex.returncode)
    except Exception as ex:
        print 'Error encountered: ' + str(ex)
    '''


def execute_subprocess_new_console(cmd):
    try:
        if os.name == "nt":
            # print ("CMD: " + cmd)
            proc = subprocess.Popen(cmd, creationflags=subprocess.CREATE_NEW_CONSOLE)
            proc.communicate()
            if proc.returncode != 0:
                raise Exception('Unexpected error when processing external command. ({0})'.format(proc.stderr))
            return proc.returncode

            '''
            returnval = os.system('start /wait cmd /c "{0}"'.format(cmd))
            print('Process return val: {0}'.format(returnval))
            return returnval
            '''

        elif os.name == "posix":
            # update me
            pass
        elif os.name == "os2":
            # update me
            pass
    except Exception as ex:
        print("Exception: Unable to execute external command. Please check command parameters.")
        raise


def execute_subprocess(cmd):
    # print('cmd: {0}'.format(cmd))
    stdout = stderr = None
    try:
        proc = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        stdout, stderr = proc.communicate()
        if stderr:
            print("Internal error: Unable to execute external command.  Please check command parameters.")
            # REMOVE ME
            raise Exception('{0}'.format(stderr))
        return stdout
    except Exception as e:
        print("Exception: Unable to execute external command. Please check command parameters.")
        raise
    '''
    except subprocess.CalledProcessError as ex:
        print 'Error encountered(subprocess): ' + str(ex.returncode)
    except Exception as ex:
        print 'Error encountered: ' + str(ex)
    '''


def inc_filename(filename):
    basename = os.path.splitext(os.path.basename(filename))[0]
    filetype = os.path.splitext(os.path.abspath(filename))[1]

    def generate_new_file(i):
        return basename + str(i) + filetype
    return generate_new_file


def check_and_get_new_name(filename):
    generatefile = inc_filename(filename)
    incval = 1

    if os.path.exists(filename):
        while True:
            if os.path.exists(generatefile(incval)):
                incval += 1
            else:
                return generatefile(incval)
    else:
        return filename


def prompt_get_choice(message, choices=[]):
    while True:
        try:
            if pythonversion2x:
                response = raw_input(message)
            else:
                response = input(message)

            if response in choices:
                return response
            elif int(response) in choices:
                return int(response)
            else:
                continue
        except:
            pass


def prompt_get_int(message):
    response = None
    while True:
        if pythonversion2x:
            response = raw_input(message)
        else:
            response = input(message)

        try:
            int(response)
            return int(response)
        except:
            pass


def prompt_get_string(message):
    response = None
    while True:
        if pythonversion2x:
            response = raw_input(message)
        else:
            response = input(message)

        if isinstance(response, basestring):
            return response


def prompt_get_reply(message):
    if pythonversion2x:
        return raw_input(message)
    else:
        return input(message)


def prompt_get_result(message, right_answers=['y', 'Y'], wrong_answers=['n', 'N']):
    while True:
        if pythonversion2x:
            response = raw_input(message)
        else:
            response = input(message)
        if response in right_answers:
            return True
        elif response in wrong_answers:
            return False
        else:
            continue


def prompt_exit_on_fail(message, right_answers=['y', 'Y'], wrong_answers=['n', 'N']):
    while True:
        if pythonversion2x:
            response = raw_input(message)
        else:
            response = input(message)
        if response in right_answers:
            break
        elif response in wrong_answers:
            exit(1)
        else:
            continue


def resource_path(filename):
    if hasattr(sys, 'frozen'):
        filepath = os.path.join(os.path.dirname(sys.executable), filename)
        if os.path.exists(filepath):
            return filepath
        filepath = os.path.join(os.path.dirname(sys.executable) + os.path.sep + _bin_folder, filename)
        if os.path.exists(filepath):
            return filepath

    filepath = os.path.join(os.getcwd(), filename)
    if os.path.exists(filepath):
        return filepath
    filepath = os.path.join(os.getcwd() + os.path.sep + _bin_folder, filename)
    if os.path.exists(filepath):
        return filepath

    filepath = os.path.join(os.path.dirname(os.path.abspath(__file__)), filename)
    if os.path.exists(filepath):
        return filepath
    filepath = os.path.join(os.path.dirname(os.path.abspath(__file__)) + os.path.sep + _bin_folder, filename)
    if os.path.exists(filepath):
        return filepath

    if hasattr(sys, '_MEIPASS'):
        filepath = os.path.join(sys._MEIPASS, filename)
        if os.path.exists(filepath):
            return filepath
        filepath = os.path.join(sys._MEIPASS + os.path.sep + _bin_folder, filename)
        if os.path.exists(filepath):
            return filepath

    if hasattr(os.environ, '_MEIPASS2'):
        filepath = os.path.join(os.environ['_MEIPASS2'], filename)
        if os.path.exists(filepath):
            return filepath
        filepath = os.path.join(os.environ['_MEIPASS2'] + os.path.sep + _bin_folder, filename)
        if os.path.exists(filepath):
            return filepath


    '''
    # elif '_MEIPASS2' in environ:
    elif os.environ['_MEIPASS2']:
        filepath = os.path.join(os.environ['_MEIPASS2'], filename)
        if os.path.exists(filepath):
            return filepath
        filepath = os.path.join(os.environ['_MEIPASS2'] + os.path.sep + _bin_folder, filename)
        if os.path.exists(filepath):
            return filepath
    '''




    raise Exception('Unable to find required file: {0}'.format(filename))


def run_executable(executable, parameters, time=None, new_console=False):
    if time and not isinstance(time, int):
        return ""
    if time is None:
        return execute_subprocess_new_console("\"" + resource_path(executable) + "\" " + parameters) if new_console \
            else execute_subprocess("\"" + resource_path(executable) + "\" " + parameters)
    else:
        return execute_subprocess_timed("\"" + resource_path(executable) + "\" " + parameters, time)




def print_exception():
    exc_type, exc_obj, tb = sys.exc_info()
    f = tb.tb_frame
    lineno = tb.tb_lineno
    filename = f.f_code.co_filename
    linecache.checkcache(filename)
    line = linecache.getline(filename, lineno, f.f_globals)
    print 'EXCEPTION IN ({}, LINE {} "{}"): {}'.format(filename, lineno, line.strip(), exc_obj)

if __name__ == '__main__':
    try:
        # execute_subprocess("C:\\XinTang\\FP_SW_000104\\trunk\\FT800 Utility\\ffprobe.exe -v quiet -print_format json -show_streams C:\\XinTang\\FP_SW_000104\\trunk\\FT800 Utility\\test.avi")
        run_executable("ffmpeg.exe", " -i adam.avi -vcodec mjpeg -b:v 10M "
                                     "-acodec pcm_mulaw -ar 44100 -ac 1 -y adam_converted.avi", None, True)
    except:
        print_exception()