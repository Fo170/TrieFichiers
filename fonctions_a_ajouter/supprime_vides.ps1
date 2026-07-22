# Supprime tous les fichiers vides dans un dossier donnée et ses sous-dossiers
$chemin = "O:\#_A_piocher\# playboy"   # <-- remplace par ton chemin

$fichiers_Vide_Supprimes = 0
$fichiers_db_Supprimes = 0
$dossiers_Vide_Supprimes = 0
Write-Host "Nettoyage du dossier : $chemin" -ForegroundColor White

Write-Host "=== Suppression des fichiers Vide ===" -ForegroundColor White

Get-ChildItem -Path $chemin -File -Recurse |
    Where-Object { $_.Length -eq 0 } |
    ForEach-Object {
        Write-Host "Suppression fichiers Vide : $($_.FullName)" -ForegroundColor Yellow
        Remove-Item -LiteralPath $_.FullName -Force
        $fichiers_Vide_Supprimes++
    }

Write-Host "=== Suppression des fichiers Thumbs.db ===" -ForegroundColor White

Get-ChildItem -Path $chemin -Filter "Thumbs.db" -Recurse -Force -ErrorAction SilentlyContinue |
    ForEach-Object {
        Write-Host "Suppression des fichiers Thumbs.db : $($_.FullName)" -ForegroundColor Cyan
        Remove-Item -LiteralPath $_.FullName -Force
        $fichiers_db_Supprimes++
    }

Write-Host "=== Suppression des dossiers vides ===" -ForegroundColor White
# On récupère d'abord les dossiers les plus profonds (sinon les parents ne sont jamais vides)
Get-ChildItem -LiteralPath $chemin -Directory -Recurse -Attributes !ReparsePoint |
    Sort-Object FullName -Descending |
    ForEach-Object {
        if (-not (Get-ChildItem -LiteralPath $_.FullName -Force)) {
            Write-Host "Suppression dossier vide : $($_.FullName)" -ForegroundColor Green
            Remove-Item -LiteralPath $_.FullName -Force
            $dossiers_Vide_Supprimes++
        }
    }

Write-Host "=== Nettoyage terminé ===" -ForegroundColor White
Write-Host "$fichiers_Vide_Supprimes fichiers vide supprimés." -ForegroundColor Yellow
Write-Host "$fichiers_db_Supprimes fichiers db supprimés." -ForegroundColor Cyan
Write-Host "$dossiers_Vide_Supprimes dossiers vide supprimés." -ForegroundColor Green