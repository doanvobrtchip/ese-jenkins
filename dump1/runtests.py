import os
import sys
import Image
import subprocess
import tempfile
import numpy as np

def main():
    ref_dir ="../reference"
    dump_dir = os.path.join(ref_dir, "dumps")
    tests = set([fn.replace(".vc1dump", "") for fn in os.listdir(dump_dir) if fn.endswith(".vc1dump")])

    # This is a list of tests to exclude.
    # Please remove passing tests from this list to prevent regressions
    notyet = set([
        'test_aa_stencil.0',
        'test_alpha_killed.0',
        'test_bm_wrap.0',
        'test_bm_xy.0',
        'test_call.0',
        'test_clear_simple.0',
        'test_edge_polygon.0',
        'test_edge_subpixel.0',
        'test_handles.0',
        'test_jump.0',
        'test_kitchensink.0',
        'test_line_extreme.0',
        'test_line_gradients.0',
        'test_line_mini.0',
        'test_line_offscreen_flat.0',
        'test_line_scatter.0',
        'test_line_scatter.1',
        'test_line_scatter.2',
        'test_line_scatter.3',
        'test_lines_offscreen.0',
        'test_lines_parse.0',
        'test_lines_subpixel.0',
        'test_lines_xy.0',
        'test_line_wide_offscreen.0',
        'test_line_width.0',
        'test_macro.0',
        'test_mem_exhaustive_b.0',
        'test_mem_exhaustive_b.1',
        'test_mem_exhaustive_b.2',
        'test_mem_exhaustive_b.3',
        'test_nonpow2.0',
        'test_outside_beginend.0',
        'test_points_increase.0',
        'test_points_large.0',
        'test_points_offscreen.0',
        'test_points_subpixel.0',
        'test_points_visit.0',
        'test_prim_scissor.0',
        'test_rects.0',
        'test_restores.0',
        'test_rom.0',
        'test_rom.1',
        'test_rr_01.0',
        'test_rr_03.0',
        'test_rr_06.0',
        'test_scissor.0',
        'test_scissor_connected.0',
        'test_scissor_max.0',
        'test_scissor_overlap.0',
        'test_shortlines.0',
        'test_state_saverestore.0',
        'test_stencil_comparisons.0',
        'test_stencil_ops.0',
        'test_textvga_1.0',
    ])

    def run1(t):
        expected = Image.open(os.path.join(dump_dir, t + ".png"))
        w,h = expected.size

        try:
            subprocess.check_call([ "./dump1/dump1", os.path.join(dump_dir, t + ".vc1dump"), "out"], stdout = open("0", "w"))
        except subprocess.CalledProcessError:
            return 'CRASH'
        bgra = Image.fromstring("RGBA", (w, h), open("out").read())
        (b,g,r,a) = bgra.split()
        actual = Image.merge("RGBA", (r,g,b,a))
        error4 = abs(np.array(actual).astype(int) - np.array(expected).astype(int))
        error = 0
        for i in range(4):
            error = np.maximum(error, error4[:,:,i])
        fails = np.argwhere(error > 32)
        if len(fails) > 0:
            expected.save("e.png")
            actual.save("a.png")
            epic = np.where(error > 32, error, 0).astype(np.uint8)
            err = Image.frombuffer("L", expected.size, epic, 'raw', "L", 0, 1).convert("RGBA")

            r = Image.new("RGBA", (3*w, h))
            r.paste(expected, (0, 0))
            r.paste(actual, (w, 0))
            r.paste(err, (2 * w, 0))
            errf = "error_%s.png" % t
            r.save(errf)
            return "%d pixels differ, differences in %s" % (len(fails), errf)
        else:
            return "pass"

    failed = []
    for t in sorted(tests - notyet):
        outcome = run1(t)
        print "%32s: %s" % (t, outcome)
        if outcome != "pass":
            failed.append(t)
    if failed:
        print "The following tests failed: " + ", ".join(failed)
    return failed

if __name__ == "__main__":
    r = main() 
    if not r:
        sys.exit(0)
    else:
        sys.exit(1) # failures, so exit nonzero
