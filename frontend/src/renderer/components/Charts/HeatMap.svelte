<script>
  import "@carbon/charts/styles.min.css";
  import "carbon-components/css/carbon-components.min.css";
  import { find, reverse, cloneDeep } from "lodash";
  import AtomicHeatMap from "@components/Charts/AtomicHeatMap.svelte";
  import { createEventDispatcher } from 'svelte';
  export let data;

  const dispatch = createEventDispatcher();

  function handleDataPointSelect(event) {
    // Re-dispatch the event to the parent
    dispatch('datapointselect', event.detail);
  }

  export let visionDeficiencyMode = false;
  let mapData = [];
  $: if (data?.matrix_values) {
    const deltaE = find(data?.matrix_values, {'name': 'CM DeltaE Values'});
    mapData = deltaE?.data.slice().reverse();
  }

  const exportCSV = function () {
    // Reverse this without mutating because it's upside down
    const flipped = reverse(cloneDeep(mapData));

    // Construct CSV by hand because there is a bug in Apexcharts relating to 2d data
    let csv = '"Row","Col","Value"\r\n';
    for(let i = 0; i < flipped.length; i++) {
      for(let j = 0; j < flipped[i].length; j++) {
        csv += [String(i+1), String.fromCharCode(j + 65), flipped[i][j]]
                .map(v => `"${v}"`)
                .join(',');
        csv += '\r\n';
      }
    }

    // Kinda jank, but found multiple sources with this strategy for
    // downloading constructed CSVs
    const blob = new Blob([csv], { type: 'text/csv;charset=utf-8;' });
    const downloadLink = document.createElement('a');
    downloadLink.href = window.URL.createObjectURL(blob);
    downloadLink.setAttribute('download', 'calibrationReport.csv');
    downloadLink.click();
  };

</script>

{#if mapData?.length > 1}
  <div class="heatmap-chart">
    ΔE Heatmap
    <button on:click={exportCSV} style="float: right; margin-left: 20px;">Export</button>
    <span style="float: right; margin-top: 2px;">
      <input type="checkbox" class="peer" bind:checked={visionDeficiencyMode} >
      <label>Grayscale</label>
    </span>
     <div style="display: flex; align-items: center;">
        <AtomicHeatMap on:datapointselect={handleDataPointSelect} data={mapData} visionDeficiencyMode={visionDeficiencyMode}></AtomicHeatMap>
     </div>
  </div>
{/if}

<style lang="postcss" global>
  .heatmap-number-grid {
    display: grid;
    grid-template-rows: repeat(calc(var(--row) - 1), auto);
    grid-template-columns: repeat(var(--col), auto);
    @apply absolute w-[75vh] h-[75vh] pt-[3rem] pb-[3.8rem] pl-[1.9rem] pointer-events-none
            top-0 left-0;
  }
  .heatmap-value {
    @apply flex justify-center items-center h-full w-full font-bold text-black;
  }
  .heatmap-chart {
    @apply relative;
  }

  /* Global Chart Styles */
  .bx--cc--chart-wrapper text {
    @apply fill-white;
  }
  .bx--cc--grid rect.chart-grid-backdrop {
    @apply fill-gray-700;
  }
  .bx--cc--axes g.axis g.tick text {
    @apply fill-white;
  }
  .bx--cc--title p.title {
    @apply text-white;
  }
  .bx--cc--axes g.axis .axis-title {
    @apply fill-white;
  }
  /* --- Tooltip */
  .bx--cc--tooltip {
    @apply bg-gray-700 text-white;
  }
  .bx--cc--tooltip .content-box {
    @apply text-white;
  }
  .bx--cc--tooltip .content-box .datapoint-tooltip p.value {
    @apply font-bold;
  }
  .bx--cc--tooltip .content-box .datapoint-tooltip {
    @apply p-2;
  }
  .bx--cc--tooltip .tooltip-color {
    @apply w-2;
  }
  /* --- */
  .bx--chart-holder {
    @apply aspect-square w-auto;
  }
  .bx--overflow-menu__icon {
    @apply fill-gray-100;
  }
  .bx--overflow-menu:hover,
  .bx--overflow-menu__trigger:hover {
    @apply bg-transparent;
  }
  .bx--overflow-menu.bx--overflow-menu--open:hover {
    @apply bg-transparent;
  }
  .bx--chart-holder .bx--cc--toolbar .bx--overflow-menu--flip.is-open {
    @apply bg-transparent;
  }
  .bx--overflow-menu-options__option:hover {
    @apply bg-transparent;
  }
  .bx--overflow-menu.bx--overflow-menu--open,
  .bx--overflow-menu.bx--overflow-menu--open .bx--overflow-menu__trigger {
    @apply bg-transparent;
  }

  .heatmap-chart .bx--cc--axes g.axis .axis-title {
    @apply hidden;
  }
</style>
