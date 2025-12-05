#!/usr/bin/env python3
"""
Create SQLite star catalog database from HYG and OpenNGC catalogs.

Downloads and processes:
- HYG Star Database (https://github.com/astronexus/HYG-Database)
- OpenNGC (https://github.com/mattiaverga/OpenNGC)
"""

import sqlite3
import csv
import os
import urllib.request
import sys

# URLs for catalog data
# HYG moved to Codeberg
HYG_URL = "https://codeberg.org/astronexus/hyg/raw/branch/main/hyg/v4/hyg_v41.csv"
OPENGC_URL = "https://raw.githubusercontent.com/mattiaverga/OpenNGC/master/database_files/NGC.csv"

DB_PATH = "stars.db"
DATA_DIR = os.path.dirname(os.path.abspath(__file__))

def download_file(url, filename):
    """Download a file if it doesn't exist."""
    filepath = os.path.join(DATA_DIR, filename)
    if os.path.exists(filepath):
        print(f"  {filename} already exists, skipping download")
        return filepath
    
    print(f"  Downloading {filename}...")
    urllib.request.urlretrieve(url, filepath)
    print(f"  Downloaded {filename}")
    return filepath

def create_database():
    """Create the SQLite database with star and DSO tables."""
    db_path = os.path.join(DATA_DIR, DB_PATH)
    
    # Remove existing database
    if os.path.exists(db_path):
        os.remove(db_path)
    
    conn = sqlite3.connect(db_path)
    cursor = conn.cursor()
    
    # Create stars table (HYG catalog)
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
    
    # Create DSO table (OpenNGC)
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
    
    # Create indices for fast queries
    cursor.execute('CREATE INDEX idx_stars_mag ON stars(mag)')
    cursor.execute('CREATE INDEX idx_stars_proper ON stars(proper)')
    cursor.execute('CREATE INDEX idx_stars_con ON stars(con)')
    cursor.execute('CREATE INDEX idx_dso_mag ON dso(mag)')
    cursor.execute('CREATE INDEX idx_dso_name ON dso(name)')
    cursor.execute('CREATE INDEX idx_dso_common ON dso(common_names)')
    
    conn.commit()
    return conn

def import_hyg_catalog(conn, filepath):
    """Import HYG star catalog."""
    print("Importing HYG star catalog...")
    cursor = conn.cursor()
    
    count = 0
    with open(filepath, 'r', encoding='utf-8') as f:
        reader = csv.DictReader(f)
        for row in reader:
            try:
                # Only import stars brighter than magnitude 10
                mag = float(row['mag']) if row['mag'] else 99
                if mag > 10:
                    continue
                
                cursor.execute('''
                    INSERT INTO stars (id, hip, hd, hr, proper, ra, dec, dist, mag, absmag, spect, ci, con, bayer, flam)
                    VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
                ''', (
                    int(row['id']) if row['id'] else None,
                    int(row['hip']) if row['hip'] else None,
                    int(row['hd']) if row['hd'] else None,
                    int(row['hr']) if row['hr'] else None,
                    row['proper'] if row['proper'] else None,
                    float(row['ra']) if row['ra'] else None,
                    float(row['dec']) if row['dec'] else None,
                    float(row['dist']) if row['dist'] else None,
                    mag,
                    float(row['absmag']) if row['absmag'] else None,
                    row['spect'] if row['spect'] else None,
                    float(row['ci']) if row['ci'] else None,
                    row['con'] if row['con'] else None,
                    row['bayer'] if row['bayer'] else None,
                    row['flam'] if row['flam'] else None
                ))
                count += 1
            except (ValueError, KeyError) as e:
                continue
    
    conn.commit()
    print(f"  Imported {count} stars")

def import_opengc_catalog(conn, filepath):
    """Import OpenNGC deep sky object catalog."""
    print("Importing OpenNGC catalog...")
    cursor = conn.cursor()
    
    # Messier catalog mapping
    messier_map = {}
    
    count = 0
    with open(filepath, 'r', encoding='utf-8') as f:
        reader = csv.DictReader(f, delimiter=';')
        for row in reader:
            try:
                name = row.get('Name', '')
                if not name:
                    continue
                
                # Parse RA (format: HH:MM:SS.s)
                ra_str = row.get('RA', '')
                if ra_str and ':' in ra_str:
                    parts = ra_str.split(':')
                    ra = float(parts[0]) * 15 + float(parts[1]) * 0.25 + float(parts[2]) * (15/3600)
                else:
                    ra = None
                
                # Parse Dec (format: +DD:MM:SS)
                dec_str = row.get('Dec', '')
                if dec_str and ':' in dec_str:
                    sign = -1 if dec_str.startswith('-') else 1
                    dec_str = dec_str.lstrip('+-')
                    parts = dec_str.split(':')
                    dec = sign * (float(parts[0]) + float(parts[1])/60 + float(parts[2])/3600)
                else:
                    dec = None
                
                if ra is None or dec is None:
                    continue
                
                mag_str = row.get('V-Mag', '') or row.get('B-Mag', '')
                mag = float(mag_str) if mag_str else None
                
                obj_type = row.get('Type', '')
                common = row.get('Common names', '')
                messier = row.get('M', '')
                
                size_major = float(row.get('MajAx', '')) if row.get('MajAx', '') else None
                size_minor = float(row.get('MinAx', '')) if row.get('MinAx', '') else None
                const = row.get('Const', '')
                
                # Add Messier designation to common names
                if messier:
                    if common:
                        common = f"M{messier}, {common}"
                    else:
                        common = f"M{messier}"
                
                cursor.execute('''
                    INSERT OR IGNORE INTO dso (name, type, ra, dec, mag, size_major, size_minor, const, common_names, messier)
                    VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
                ''', (name, obj_type, ra, dec, mag, size_major, size_minor, const, common, messier))
                count += 1
                
            except (ValueError, KeyError) as e:
                continue
    
    conn.commit()
    print(f"  Imported {count} deep sky objects")

def add_common_objects(conn):
    """Add some well-known objects with common names."""
    print("Adding common object aliases...")
    cursor = conn.cursor()
    
    # Update common names for famous objects
    updates = [
        ("NGC0224", "Andromeda Galaxy"),
        ("NGC5194", "Whirlpool Galaxy"),
        ("NGC7293", "Helix Nebula"),
        ("NGC1976", "Orion Nebula"),
        ("NGC6720", "Ring Nebula"),
        ("NGC6853", "Dumbbell Nebula"),
        ("NGC5139", "Omega Centauri"),
        ("NGC104", "47 Tucanae"),
        ("NGC6341", "M92"),
        ("NGC7078", "M15"),
    ]
    
    for name, common in updates:
        cursor.execute('''
            UPDATE dso SET common_names = COALESCE(common_names || ', ', '') || ?
            WHERE name = ? AND (common_names IS NULL OR common_names NOT LIKE ?)
        ''', (common, name, f'%{common}%'))
    
    conn.commit()

def main():
    print("Creating star catalog database...")
    print()
    
    # Download catalogs
    print("Step 1: Downloading catalogs...")
    hyg_file = download_file(HYG_URL, "hyg_v41.csv")
    ngc_file = download_file(OPENGC_URL, "NGC.csv")
    print()
    
    # Create database
    print("Step 2: Creating database...")
    conn = create_database()
    print()
    
    # Import data
    print("Step 3: Importing data...")
    import_hyg_catalog(conn, hyg_file)
    import_opengc_catalog(conn, ngc_file)
    add_common_objects(conn)
    print()
    
    # Statistics
    cursor = conn.cursor()
    cursor.execute("SELECT COUNT(*) FROM stars")
    star_count = cursor.fetchone()[0]
    cursor.execute("SELECT COUNT(*) FROM dso")
    dso_count = cursor.fetchone()[0]
    
    print(f"Database created: {os.path.join(DATA_DIR, DB_PATH)}")
    print(f"  Stars: {star_count}")
    print(f"  Deep Sky Objects: {dso_count}")
    
    conn.close()
    print("\nDone!")

if __name__ == "__main__":
    main()
