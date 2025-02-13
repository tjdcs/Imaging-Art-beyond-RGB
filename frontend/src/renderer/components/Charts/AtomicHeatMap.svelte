<script>
    import { chart } from "svelte-apexcharts";
    import { createEventDispatcher } from 'svelte';

    const dispatch = createEventDispatcher();

    export let data;

    // Colorblind option
    export let visionDeficiencyMode = false;

    // Parse the needed data for the heatmap
    function getData() {
        let formattedData = [];
        let tempData = [];
        for (let i = 0; i < data.length; i++) {
            tempData[i] = [];
            for (let j = 0; j < data[i].length; j++) {
                tempData[i][j] = {
                    y: data[i][j].toFixed(2),
                    x: String.fromCharCode(j + 65)
                };
            }
            formattedData.push({
                name: i,
                data: tempData[i]
            });
        }

        return formattedData;
    }

    const colors = [
        "#0a9400",
        "#199a00",
        "#2a9f00",
        "#3ca500",
        "#4eab00",
        "#62b000",
        "#77b600",
        "#8dbb00",
        "#a4c100",
        "#bdc700",
        "#ccc300",
        "#d2b300",
        "#d8a300",
        "#dd9200",
        "#e37f00",
        "#e86c00",
        "#ee5700",
        "#f44100",
        "#f92a00",
        "#ff1200"
    ];

    const grayScale = [
        "#ffffff",
        "#efefef",
        "#d3d3d3",
        "#c1c1c1",
        "#b5b5b5",
        "#9e9e9e",
        "#898989",
        "#767676",
        "#585858",
        "#4c4c4c",
        "#4a4a4a",
        "#4c4c4c",
        "#383838",
        "#383838",
        "#313131",
        "#2a2a2a",
        "#1c1c1c",
        "#191919",
        "#171717",
        "#000000"
    ];

    function ranges() {
        const result = [];
        for (let i = 0; i < 20; i++) {
            result.push({
                from: i/2,
                to: i/2 + .5,
                color: visionDeficiencyMode ? grayScale[i] : colors[i]
            });
        }
        result[result.length - 1].to = Number.MAX_SAFE_INTEGER;
        return result;
    }
	
function generateLegendRanges() {
    const rangeData = ranges(); // Get full color ranges
    const result = [];

    
    const step = 2; // Merge every 2 ranges to create 10 values

    for (let i = 0; i < 10; i++) {
        let from = rangeData[i * step].from;
        let to = rangeData[i * step + step - 1]?.to || rangeData[rangeData.length - 1].to; // Make sure range goes to 10

        result.push({
            label: (i + 1).toString(), // Display whole numbers 1-10
            color: rangeData[i * step].color // Use color from paired ranges
        });
    }

    return result;
}
    const getOptions = function() {
        return {
            series: getData(),
            chart: {
                events: {
                    dataPointSelection: function(event, chartContext, config) {
                        let yAxisLabel = data.length - config.seriesIndex;
                        let xValue = config.w.config.series[config.seriesIndex].data[config.dataPointIndex].x;
                        console.log(yAxisLabel)
                        console.log(xValue)
						dispatch('datapointselect', { yAxisLabel, xValue });
                        // Dispatch a custom event with the selected data point information
                        //dispatch('datapointselect', { yAxisLabel, xValue });
                    }
                },
                height: '650px',
                width: '650px',
                type: 'heatmap',
                toolbar: {
                    // Hamburger menu which has exports such as CSV etc.
                    // I have had issues displaying this, I believe some unrelated global CSS is causing issues
                    show: false
                },
                selection: {
                    enabled: true
                }
            },
            legend: {
                show: true,
				position: 'top',
				horizontalAlign: 'center', 
				customLegendItems: generateLegendRanges().map(range => range.label),
				markers: {
				    fillColors: generateLegendRanges().map(range => range.color)
				},
				labels: {
				    colors: "#ffffff",
                    formatter: function(value, index) {
                       let legendRanges = generateLegendRanges();
                       return legendRanges[index]?.label || "";

					}
				},
				onItemClick: {
                toggleDataSeries: false 
                },
                 onItemHover: {
                 highlightDataSeries: true 
    }
            },
            tooltip: {
                enabled: true,
                theme: 'dark',
                y: {
                    title: { //todo keep????? y axis tooltip
                        // Displayed before the value on the tooltip, unnecessary
                        formatter: (value) => ""//{return data?.length-value}
                    }
                },
                
            },
            yaxis: {
                labels: {
                    formatter: function (value, info) {
                        if(info === data?.length) {
                            // This gets hit before we even get to the heatmap and displays a random axisLabel
                            return "";
                        } else if(isFinite(info)){
                            // This gets hit for each row on the heatmap (displayed as y-axis)
                            return data?.length - info;
                        } else {
                            // This gets hit for each individual square on the heatmap (displayed as tooltip value)
                            return value;
                        }
                    },
                    style: {
                        colors: ["#ffffff"]   
                    },
                },
                
            },
            xaxis: {
                labels: {
                    style: {
                        //for some reason colors: ["#ffffff", "#ffffff"] would only make the first two labels white??
                        //so make an array the length of the x-axis of just "#ffffff"
                        colors:  Array.from({length:data?.[0].length}, ()=> "#ffffff")
                    }
                }
            },
            plotOptions: {
                heatmap: {
                    shadeIntensity: 0.5,
                    radius: 0,
                    useFillColorAsStroke: true,
                    // EnableShades ends up shading higher-value greens darker shades, which is not what we want
                    enableShades: false,
                    colorScale: {
                        ranges: ranges()
                    }
                }
            },
            dataLabels: {
                enabled: true,
                style: {
                    colors: visionDeficiencyMode ? ['#e60b0b'] : ['#ffffff']
                }
            }
        }
    };

</script>

<main>
    {#key visionDeficiencyMode}
        <div use:chart={getOptions()}></div>
    {/key}
</main>
