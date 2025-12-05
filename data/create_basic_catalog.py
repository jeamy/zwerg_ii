#!/usr/bin/env python3
"""
Create a basic star catalog with the most important celestial objects.
This is a fallback when online catalogs are not available.
"""

import sqlite3
import os

DB_PATH = "stars.db"
DATA_DIR = os.path.dirname(os.path.abspath(__file__))

# Famous stars with their coordinates
FAMOUS_STARS = [
    # id, name, ra (hours), dec (degrees), magnitude, constellation
    (1, "Sirius", 6.752, -16.716, -1.46, "CMa"),
    (2, "Canopus", 6.399, -52.696, -0.74, "Car"),
    (3, "Arcturus", 14.261, 19.182, -0.05, "Boo"),
    (4, "Vega", 18.616, 38.784, 0.03, "Lyr"),
    (5, "Capella", 5.278, 45.998, 0.08, "Aur"),
    (6, "Rigel", 5.242, -8.202, 0.13, "Ori"),
    (7, "Procyon", 7.655, 5.225, 0.34, "CMi"),
    (8, "Betelgeuse", 5.919, 7.407, 0.42, "Ori"),
    (9, "Altair", 19.846, 8.868, 0.76, "Aql"),
    (10, "Aldebaran", 4.599, 16.509, 0.85, "Tau"),
    (11, "Antares", 16.490, -26.432, 0.96, "Sco"),
    (12, "Spica", 13.420, -11.161, 0.97, "Vir"),
    (13, "Pollux", 7.755, 28.026, 1.14, "Gem"),
    (14, "Fomalhaut", 22.961, -29.622, 1.16, "PsA"),
    (15, "Deneb", 20.690, 45.280, 1.25, "Cyg"),
    (16, "Regulus", 10.139, 11.967, 1.35, "Leo"),
    (17, "Castor", 7.577, 31.888, 1.58, "Gem"),
    (18, "Polaris", 2.530, 89.264, 1.98, "UMi"),
    (19, "Bellatrix", 5.419, 6.350, 1.64, "Ori"),
    (20, "Alnilam", 5.603, -1.202, 1.69, "Ori"),
    (21, "Alnitak", 5.679, -1.943, 1.77, "Ori"),
    (22, "Mintaka", 5.533, -0.299, 2.23, "Ori"),
    (23, "Dubhe", 11.062, 61.751, 1.79, "UMa"),
    (24, "Merak", 11.031, 56.382, 2.37, "UMa"),
    (25, "Alioth", 12.900, 55.960, 1.77, "UMa"),
    (26, "Mizar", 13.399, 54.925, 2.27, "UMa"),
    (27, "Alkaid", 13.792, 49.313, 1.86, "UMa"),
    (28, "Schedar", 0.675, 56.537, 2.23, "Cas"),
    (29, "Mirfak", 3.405, 49.861, 1.79, "Per"),
    (30, "Algol", 3.136, 40.957, 2.12, "Per"),
]

# Messier objects and other famous DSOs
FAMOUS_DSOS = [
    # name, common_name, type, ra (degrees), dec (degrees), magnitude, constellation
    ("M1", "Crab Nebula", "SNR", 83.633, 22.015, 8.4, "Tau"),
    ("M13", "Hercules Cluster", "GC", 250.423, 36.460, 5.8, "Her"),
    ("M27", "Dumbbell Nebula", "PN", 299.902, 22.721, 7.5, "Vul"),
    ("M31", "Andromeda Galaxy", "Gx", 10.685, 41.269, 3.4, "And"),
    ("M33", "Triangulum Galaxy", "Gx", 23.462, 30.660, 5.7, "Tri"),
    ("M42", "Orion Nebula", "Nb", 83.822, -5.391, 4.0, "Ori"),
    ("M44", "Beehive Cluster", "OC", 130.025, 19.621, 3.7, "Cnc"),
    ("M45", "Pleiades", "OC", 56.601, 24.114, 1.6, "Tau"),
    ("M51", "Whirlpool Galaxy", "Gx", 202.470, 47.195, 8.4, "CVn"),
    ("M57", "Ring Nebula", "PN", 283.396, 33.029, 8.8, "Lyr"),
    ("M81", "Bode's Galaxy", "Gx", 148.888, 69.065, 6.9, "UMa"),
    ("M82", "Cigar Galaxy", "Gx", 148.968, 69.680, 8.4, "UMa"),
    ("M101", "Pinwheel Galaxy", "Gx", 210.802, 54.349, 7.9, "UMa"),
    ("M104", "Sombrero Galaxy", "Gx", 189.998, -11.623, 8.0, "Vir"),
    ("NGC869", "Double Cluster h", "OC", 34.750, 57.133, 5.3, "Per"),
    ("NGC884", "Double Cluster chi", "OC", 35.600, 57.150, 6.1, "Per"),
    ("NGC7000", "North America Nebula", "Nb", 314.000, 44.333, 4.0, "Cyg"),
    ("IC434", "Horsehead Nebula", "Nb", 85.250, -2.458, 6.8, "Ori"),
    ("NGC6960", "Veil Nebula West", "SNR", 312.750, 30.717, 7.0, "Cyg"),
    ("NGC6992", "Veil Nebula East", "SNR", 314.000, 31.717, 7.0, "Cyg"),
    ("M8", "Lagoon Nebula", "Nb", 271.100, -24.383, 6.0, "Sgr"),
    ("M16", "Eagle Nebula", "Nb", 274.700, -13.783, 6.0, "Ser"),
    ("M17", "Omega Nebula", "Nb", 275.200, -16.183, 6.0, "Sgr"),
    ("M20", "Trifid Nebula", "Nb", 270.600, -23.033, 6.3, "Sgr"),
    ("M22", "Sagittarius Cluster", "GC", 279.100, -23.900, 5.1, "Sgr"),
    ("M3", "Globular Cluster", "GC", 205.548, 28.377, 6.2, "CVn"),
    ("M5", "Globular Cluster", "GC", 229.638, 2.081, 5.6, "Ser"),
    ("M15", "Pegasus Cluster", "GC", 322.493, 12.167, 6.2, "Peg"),
    ("M92", "Globular Cluster", "GC", 259.281, 43.136, 6.4, "Her"),
    ("NGC253", "Sculptor Galaxy", "Gx", 11.888, -25.288, 7.1, "Scl"),
    ("NGC2237", "Rosette Nebula", "Nb", 98.000, 5.000, 9.0, "Mon"),
    ("NGC2024", "Flame Nebula", "Nb", 85.417, -1.850, 7.0, "Ori"),
    ("NGC7293", "Helix Nebula", "PN", 337.411, -20.837, 7.3, "Aqr"),
    ("NGC6543", "Cat's Eye Nebula", "PN", 269.639, 66.633, 8.1, "Dra"),
    ("NGC2392", "Eskimo Nebula", "PN", 112.292, 20.912, 9.2, "Gem"),
]

def create_database():
    """Create the SQLite database."""
    db_path = os.path.join(DATA_DIR, DB_PATH)
    
    if os.path.exists(db_path):
        os.remove(db_path)
    
    conn = sqlite3.connect(db_path)
    cursor = conn.cursor()
    
    # Create stars table
    cursor.execute('''
        CREATE TABLE stars (
            id INTEGER PRIMARY KEY,
            hip INTEGER,
            hd INTEGER,
            hr INTEGER,
            proper TEXT,
            ra REAL,
            dec REAL,
            dist REAL,
            mag REAL,
            absmag REAL,
            spect TEXT,
            ci REAL,
            con TEXT,
            bayer TEXT,
            flam TEXT
        )
    ''')
    
    # Create DSO table
    cursor.execute('''
        CREATE TABLE dso (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT UNIQUE,
            type TEXT,
            ra REAL,
            dec REAL,
            mag REAL,
            size_major REAL,
            size_minor REAL,
            const TEXT,
            common_names TEXT,
            messier TEXT
        )
    ''')
    
    # Create indices
    cursor.execute('CREATE INDEX idx_stars_mag ON stars(mag)')
    cursor.execute('CREATE INDEX idx_stars_proper ON stars(proper)')
    cursor.execute('CREATE INDEX idx_dso_name ON dso(name)')
    cursor.execute('CREATE INDEX idx_dso_common ON dso(common_names)')
    
    conn.commit()
    return conn

def import_stars(conn):
    """Import famous stars."""
    cursor = conn.cursor()
    
    for star in FAMOUS_STARS:
        id_, name, ra_hours, dec, mag, con = star
        ra_deg = ra_hours * 15  # Convert hours to degrees
        cursor.execute('''
            INSERT INTO stars (id, proper, ra, dec, mag, con)
            VALUES (?, ?, ?, ?, ?, ?)
        ''', (id_, name, ra_deg, dec, mag, con))
    
    conn.commit()
    print(f"Imported {len(FAMOUS_STARS)} stars")

def import_dsos(conn):
    """Import famous deep sky objects."""
    cursor = conn.cursor()
    
    for dso in FAMOUS_DSOS:
        name, common, obj_type, ra, dec, mag, con = dso
        messier = name if name.startswith("M") else None
        cursor.execute('''
            INSERT INTO dso (name, type, ra, dec, mag, const, common_names, messier)
            VALUES (?, ?, ?, ?, ?, ?, ?, ?)
        ''', (name, obj_type, ra, dec, mag, con, common, messier))
    
    conn.commit()
    print(f"Imported {len(FAMOUS_DSOS)} deep sky objects")

def main():
    print("Creating basic star catalog...")
    
    conn = create_database()
    import_stars(conn)
    import_dsos(conn)
    
    db_path = os.path.join(DATA_DIR, DB_PATH)
    print(f"\nDatabase created: {db_path}")
    
    conn.close()
    print("Done!")

if __name__ == "__main__":
    main()
