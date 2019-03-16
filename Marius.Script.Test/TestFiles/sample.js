var Chars = " .,:;=|iI+hHOE#$-";

for (Im = -1.2; Im <= 1.2; Im += .05) {
    var line = '';
    for (Re = -2; Re <= 1; Re += .03) {
        var Zr = Re;
        var Zi = Im;
        var n = 0;
        for (; n < 16; n++) {
            var Zr2 = Zr * Zr;
            var Zi2 = Zi * Zi;
            if (Zr2 + Zi2 > 4) break;
            Zi = 2 * Zr * Zi + Im; Zr = Zr2 - Zi2 + Re;
        }
        outline(Chars[n]);
    }
    printLine(line);
}

function printLine(line) {
    outline(line);
    outline('\r\n');
}
