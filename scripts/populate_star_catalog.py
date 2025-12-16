#!/usr/bin/env python3
"""
Populate the star catalog database with HYG Star Catalog and OpenNGC data.
Downloads data from GitHub and imports into SQLite.
"""

import sqlite3
import csv
import urllib.request
import os
import sys

DB_PATH = os.path.join(os.path.dirname(__file__), '..', 'data', 'stars.db')

# HYG Star Catalog v3.7 - ~120,000 stars
HYG_URL = "https://raw.githubusercontent.com/astronexus/HYG-Database/master/hyg/v3/hyg_v37.csv"

# OpenNGC - ~13,000 deep sky objects
OPENNGC_URL = "https://raw.githubusercontent.com/mattiaverga/OpenNGC/master/NGC.csv"

def download_file(url, local_path):
    """Download a file with progress indication."""
    print(f"Downloading {url}...")
    try:
        urllib.request.urlretrieve(url, local_path)
        print(f"  -> Saved to {local_path}")
        return True
    except Exception as e:
        print(f"  -> Error: {e}")
        return False

def create_tables(conn):
    """Create database tables if they don't exist."""
    cursor = conn.cursor()
    
    # Drop existing tables to start fresh
    cursor.execute("DROP TABLE IF EXISTS stars")
    cursor.execute("DROP TABLE IF EXISTS dso")
    
    cursor.execute("""
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
    """)
    
    cursor.execute("""
        CREATE TABLE dso (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT,
            ra REAL,
            dec REAL,
            mag REAL,
            type TEXT,
            const TEXT,
            common_names TEXT,
            size_major REAL,
            size_minor REAL
        )
    """)
    
    # Create indexes for fast queries
    cursor.execute("CREATE INDEX idx_stars_mag ON stars(mag)")
    cursor.execute("CREATE INDEX idx_stars_proper ON stars(proper)")
    cursor.execute("CREATE INDEX idx_stars_con ON stars(con)")
    cursor.execute("CREATE INDEX idx_dso_mag ON dso(mag)")
    cursor.execute("CREATE INDEX idx_dso_name ON dso(name)")
    cursor.execute("CREATE INDEX idx_dso_type ON dso(type)")
    
    conn.commit()
    print("Created database tables")

def import_hyg_catalog(conn, csv_path):
    """Import HYG star catalog into database."""
    cursor = conn.cursor()
    
    count = 0
    with open(csv_path, 'r', encoding='utf-8') as f:
        reader = csv.DictReader(f)
        
        for row in reader:
            try:
                # Skip stars without valid magnitude
                mag_str = row.get('mag', '')
                if not mag_str:
                    continue
                mag = float(mag_str)
                
                # Only import stars brighter than magnitude 10
                if mag > 10.0:
                    continue
                
                # Parse RA (in hours) and Dec (in degrees)
                ra_str = row.get('ra', '')
                dec_str = row.get('dec', '')
                if not ra_str or not dec_str:
                    continue
                
                ra = float(ra_str)  # Already in hours
                dec = float(dec_str)
                
                # Parse optional fields
                hip = int(row.get('hip', 0) or 0)
                hd = int(row.get('hd', 0) or 0)
                hr = int(row.get('hr', 0) or 0)
                proper = row.get('proper', '') or ''
                dist_str = row.get('dist', '')
                dist = float(dist_str) if dist_str else None
                absmag_str = row.get('absmag', '')
                absmag = float(absmag_str) if absmag_str else None
                spect = row.get('spect', '') or ''
                ci_str = row.get('ci', '')
                ci = float(ci_str) if ci_str else None
                con = row.get('con', '') or ''
                bayer = row.get('bayer', '') or ''
                flam = row.get('flam', '') or ''
                
                cursor.execute("""
                    INSERT INTO stars (hip, hd, hr, proper, ra, dec, dist, mag, absmag, spect, ci, con, bayer, flam)
                    VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
                """, (hip, hd, hr, proper, ra, dec, dist, mag, absmag, spect, ci, con, bayer, flam))
                
                count += 1
                if count % 5000 == 0:
                    print(f"  Imported {count} stars...")
                    
            except (ValueError, KeyError) as e:
                continue
    
    conn.commit()
    print(f"Imported {count} stars from HYG catalog")
    return count

def import_openngc(conn, csv_path):
    """Import OpenNGC deep sky objects into database."""
    cursor = conn.cursor()
    
    count = 0
    with open(csv_path, 'r', encoding='utf-8') as f:
        reader = csv.DictReader(f, delimiter=';')
        
        for row in reader:
            try:
                name = row.get('Name', '')
                if not name:
                    continue
                
                # Parse RA (format: HH:MM:SS.s)
                ra_str = row.get('RA', '')
                if not ra_str:
                    continue
                ra_parts = ra_str.split(':')
                if len(ra_parts) >= 2:
                    ra_hours = float(ra_parts[0])
                    ra_mins = float(ra_parts[1])
                    ra_secs = float(ra_parts[2]) if len(ra_parts) > 2 else 0
                    ra = ra_hours + ra_mins/60 + ra_secs/3600  # In hours
                    ra_deg = ra * 15.0  # Convert to degrees
                else:
                    continue
                
                # Parse Dec (format: +DD:MM:SS)
                dec_str = row.get('Dec', '')
                if not dec_str:
                    continue
                dec_sign = 1 if dec_str[0] != '-' else -1
                dec_str = dec_str.lstrip('+-')
                dec_parts = dec_str.split(':')
                if len(dec_parts) >= 2:
                    dec_deg = float(dec_parts[0])
                    dec_mins = float(dec_parts[1])
                    dec_secs = float(dec_parts[2]) if len(dec_parts) > 2 else 0
                    dec = dec_sign * (dec_deg + dec_mins/60 + dec_secs/3600)
                else:
                    continue
                
                # Magnitude (V-Mag or B-Mag)
                mag_str = row.get('V-Mag', '') or row.get('B-Mag', '')
                mag = float(mag_str) if mag_str else None
                
                # Type
                obj_type = row.get('Type', '') or ''
                
                # Constellation
                const = row.get('Const', '') or ''
                
                # Common names
                common_names = row.get('Common names', '') or ''
                
                # Size
                size_major_str = row.get('MajAx', '')
                size_major = float(size_major_str) if size_major_str else None
                size_minor_str = row.get('MinAx', '')
                size_minor = float(size_minor_str) if size_minor_str else None
                
                cursor.execute("""
                    INSERT INTO dso (name, ra, dec, mag, type, const, common_names, size_major, size_minor)
                    VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)
                """, (name, ra_deg, dec, mag, obj_type, const, common_names, size_major, size_minor))
                
                count += 1
                if count % 1000 == 0:
                    print(f"  Imported {count} DSOs...")
                    
            except (ValueError, KeyError) as e:
                continue
    
    conn.commit()
    print(f"Imported {count} deep sky objects from OpenNGC")
    return count

def main():
    # Ensure data directory exists
    data_dir = os.path.dirname(DB_PATH)
    os.makedirs(data_dir, exist_ok=True)
    
    # Download catalogs
    hyg_path = os.path.join(data_dir, 'hyg_v37.csv')
    ngc_path = os.path.join(data_dir, 'NGC.csv')
    
    if not os.path.exists(hyg_path):
        if not download_file(HYG_URL, hyg_path):
            print("Failed to download HYG catalog")
            sys.exit(1)
    else:
        print(f"Using existing HYG catalog: {hyg_path}")
    
    if not os.path.exists(ngc_path):
        if not download_file(OPENNGC_URL, ngc_path):
            print("Failed to download OpenNGC catalog")
            sys.exit(1)
    else:
        print(f"Using existing OpenNGC catalog: {ngc_path}")
    
    # Connect to database
    print(f"\nOpening database: {DB_PATH}")
    conn = sqlite3.connect(DB_PATH)
    
    # Create tables
    create_tables(conn)
    
    # Import data
    print("\nImporting HYG Star Catalog...")
    star_count = import_hyg_catalog(conn, hyg_path)
    
    print("\nImporting OpenNGC...")
    dso_count = import_openngc(conn, ngc_path)
    
    # Summary
    print(f"\n=== Summary ===")
    print(f"Stars: {star_count}")
    print(f"Deep Sky Objects: {dso_count}")
    print(f"Database: {DB_PATH}")
    
    conn.close()
    print("\nDone!")

if __name__ == '__main__':
    main()
