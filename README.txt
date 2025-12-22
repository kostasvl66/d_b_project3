Υλοποιήσεις:
Βλαζάκης Κωνσταντίνος-Γεώργιος (ΑΜ: 115202300017): CHUNK_CreateIterator(), CHUNK_GetNext(), CHUNK_CreateRecordIterator(), CHUNK_GetNextRecord(), merge()
Οζίνης Εμμανουήλ-Ταξιάρχης (ΑΜ:1115202300147): CHUNK_GetIthRecordInChunk(), CHUNK_UpdateIthRecord(), CHUNK_Print(), shouldSwap(), sort_FileInChunks(), sort_Chunk()

Το LLM που χρησιμοποιήθηκε για την υλοποίηση όλων των συναρτήσεων είναι το ChatGPT.

Σχεδιαστικές επιλογές - παραδοχές:
Στην βιβλιοθήκη CHUNK χρησιμοποιείται η δεδομένη οργάνωση, με ελάχιστες αλλαγές και πιθανές διαφορές στις παραδοχές. Συγκεκριμένα οι αλλαγές που έγιναν πάνω στην βιβλιοθήκη CHUNK όπως δίνεται είναι:
- Στην δομή CHUNK_Iterator το πεδίο `int current` έχει αντικατασταθεί από πεδίο `int firstBlockID`, που αντιστοιχεί πρώτο block id του αρχικού CHUNK.
- Σύμβαση ότι στην δομη CHUNK_Iterator το πεδίο `int lastBlockID` είναι inclusive για το τελευταίο block ενός CHUNK.
- Σύμβαση ότι για την δομή CHUNK τα `int from_BlockId` και `int to_BlockId` είναι inclusive.
- Επειδή σε κάποιες συναρτήσεις δεν είχε οριστεί στην δεδομένη βιβλιοθήκη, για όσες συναρτήσεις επιστρέφουν ακέραιο για success/failure, η σύμβαση είναι 0 success, -1 failure. Στην περίπτωση των συναρτήσεων CHUNK_GetNext και CHUNK_GetNextRecord, -1 σημαίνει και οτί ο iterator έφτασε στο τέλος.

Στην βιβλιοθήκη HP, επειδή οι συμβάσεις για success/failure return values δεν αναφέρονται παντού, αλλά και επειδή διαφέρουν ελαφρώς από συνάρτηση σε συνάρτηση, στα prompts αναφέρεται η παραδοχή ότι -1 σημαίνει failure και οτιδήποτε άλλο είναι success. Αυτή η παραδοχή είναι συμβατή με όσες προδιαγραφές των συναρτήσεων HP αναφέρουν success/failure return values, και πιθανότατα συμβατές με όσες δεν αναφέρονται. Προφανώς όλα αυτά ισχύουν μόνο σε όσες συναρτήσεις επιστρέφουν int για λόγους validation και όχι ως δεδομένα.
